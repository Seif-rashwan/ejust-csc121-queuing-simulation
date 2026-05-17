// server.js
// Node.js orchestration layer, spawns the C++ simulation binary,
// buffers STATE output, and serves it to the browser via REST.

import { fileURLToPath } from "node:url";
import { spawn } from "node:child_process";
import express from "express";
import path from "node:path";
import cors from "cors";
import fs from "node:fs";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
const PORT = 8081;

app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, "..", "frontend")));

// ────────────────────────────────────────────────────────────────────────────
// Runtime State:
// ──────────────
let simulationProcess = null;
let stateQueue = [];
let paused = false;
let starting = false;

const MAX_QUEUE_SIZE = 100_000;
const WATCHDOG_TIMEOUT = 30_000; // 30 seconds
const WATCHDOG_ESCALATE = 5_000; // Try SIGTERM, escalate to SIGKILL after 5s
const exeName = process.platform === "win32" ? "simulation.exe" : "simulation";
const cppExecutable = path.join(__dirname, "..", "build", "bin", exeName);

let config = {
  servers: 4,
  arrivalMin: 2,
  arrivalMax: 5,
  serviceMin: 4,
  serviceMax: 8,
  maxCustomers: 100,
  maxQueueSize: 50,
};

let simulationState = blankState();
function blankState() {
  return {
    tick: 0,
    queueSize: 0,
    served: 0,
    avgWait: 0,
    nextArrival: 0,
    running: false,
    turnedAway: 0,
    peakQueue: 0,
    lastEvent: "",
    lastEventCustomer: -1,
    totalCustomers: config.maxCustomers,
    arrived: 0,
    throughput: 0,
    _front: 0,
    _rear: 0,
    _fifoSlots: [],
    servers: blankServers(),
  };
}

function blankServers() {
  return Array.from({ length: config.servers }, () => ({
    busy: false,
    remaining: 0,
    customerId: -1,
  }));
}

// ────────────────────────────────────────────────────────────────────────────
// Routes:
// ───────
app.get("/api/state", (req, res) => {
  if (!paused && stateQueue.length > 0) {
    simulationState = { ...simulationState, ...stateQueue.shift() };
  }
  res.json(simulationState);
});

app.post("/api/config", (req, res) => {
  const { servers, arrivalMin, arrivalMax, serviceMin, serviceMax, maxCustomers, maxQueueSize } =
    req.body;

  if (
    typeof servers !== "number" ||
    servers < 1 ||
    typeof arrivalMin !== "number" ||
    arrivalMin < 1 ||
    typeof arrivalMax !== "number" ||
    arrivalMax < arrivalMin ||
    typeof serviceMin !== "number" ||
    serviceMin < 1 ||
    typeof serviceMax !== "number" ||
    serviceMax < serviceMin ||
    typeof maxCustomers !== "number" ||
    maxCustomers < 1 ||
    (maxQueueSize !== undefined && (typeof maxQueueSize !== "number" || maxQueueSize < 1))
  ) {
    res.status(400).json({ success: false, message: "Invalid config parameters." });
    return;
  }

  config = {
    servers,
    arrivalMin,
    arrivalMax,
    serviceMin,
    serviceMax,
    maxCustomers,
    maxQueueSize: maxQueueSize ?? config.maxQueueSize,
  };
  console.log("Config updated:", config);
  res.json({ success: true });
});

app.post("/api/start", (_, res) => {
  if (starting) {
    res.status(429).json({ success: false, message: "Already starting." });
    return;
  }

  if (!fs.existsSync(cppExecutable)) {
    res.status(500).json({
      success: false,
      message: `Binary not found at ${cppExecutable}. Run 'make build' first.`,
    });
    return;
  }

  if (simulationProcess && !simulationProcess.killed) {
    simulationProcess.kill();
    simulationProcess = null;
  }

  const safetyTime = config.maxCustomers * Math.max(config.arrivalMax, config.serviceMax) * 10;

  // argv layout: matches SimulationEngine web mode:
  // [1] safetyTimeCap  [2] servers
  // [3] transMin       [4] transMax
  // [5] arrivalMin     [6] arrivalMax
  // [7] totalCustomers [8] maxQueueSize
  const args = [
    safetyTime.toString(),
    config.servers.toString(),
    config.serviceMin.toString(),
    config.serviceMax.toString(),
    config.arrivalMin.toString(),
    config.arrivalMax.toString(),
    config.maxCustomers.toString(),
    config.maxQueueSize.toString(),
  ];

  stateQueue = [];
  paused = false;
  starting = true;

  simulationProcess = spawn(cppExecutable, args);
  let lineBuffer = "";

  // Primary watchdog: attempt graceful termination (SIGTERM on Unix, terminate on Windows)
  const watchdog = setTimeout(() => {
    if (!(simulationProcess && !simulationProcess.killed)) {
      return;
    }

    console.warn(
      `[Warning] Simulation exceeded ${WATCHDOG_TIMEOUT}ms timeout. Initiating graceful shutdown...`,
    );

    simulationProcess.kill();

    // Escalation timer: force kill if graceful shutdown fails
    const escalate = setTimeout(() => {
      if (simulationProcess && !simulationProcess.killed) {
        console.error("[Warning] Graceful shutdown failed. Attempting force kill...");
        try {
          simulationProcess.kill("SIGKILL"); // Best effort on Unix-like systems
        } catch (e) {
          console.warn("[Warning] Force kill not available on this platform.");
        }
        simulationState.running = false;
        starting = false;
      }
    }, WATCHDOG_ESCALATE);

    // Clear escalation timer if process exits normally
    if (simulationProcess) {
      simulationProcess.once("exit", () => clearTimeout(escalate));
    }
  }, WATCHDOG_TIMEOUT);

  simulationProcess.stdout.on("data", (data) => {
    lineBuffer += data.toString();
    const lines = lineBuffer.split("\n");
    lineBuffer = lines.pop() ?? "";
    lines.forEach((line) => {
      line = line.trim();
      if (line.length === 0) return;
      if (line.startsWith("STATE:") && stateQueue.length >= MAX_QUEUE_SIZE) {
        if (stateQueue.length === MAX_QUEUE_SIZE) {
          console.warn(
            `[Warning] Node.js stateQueue hit MAX_QUEUE_SIZE (${MAX_QUEUE_SIZE}). Dropping frames!`,
          );
          stateQueue.push({ dropped: true });
        }
        return;
      }
      parseLine(line);
    });
  });

  simulationProcess.stderr.on("data", (data) => {
    console.error("Simulation stderr:", data.toString());
  });

  simulationProcess.on("error", (err) => {
    console.error("Spawn failed:", err.message);
    simulationState.running = false;
    starting = false;
  });

  simulationProcess.on("close", (code) => {
    clearTimeout(watchdog);
    console.log(`Simulation exited with code ${code}`);
    try {
      if (lineBuffer.trim().length > 0) parseLine(lineBuffer.trim());
    } catch (e) {
      // Ignore partial json parse errors at the very end
    }
    lineBuffer = "";
    simulationState.running = false;
    starting = false;
  });

  simulationState.running = true;
  res.json({ success: true });
});

app.post("/api/pause", (_, res) => {
  paused = !paused;
  simulationState.running = !paused;
  res.json({ success: true, running: !paused });
});

app.post("/api/reset", (_, res) => {
  if (simulationProcess && !simulationProcess.killed) {
    simulationProcess.kill();
    simulationProcess = null;
  }

  stateQueue = [];
  paused = false;
  starting = false;
  simulationState = blankState();

  res.json({ success: true });
});

// ────────────────────────────────────────────────────────────────────────────
// Output Parser:
// ──────────────
/**
 * Parses one complete stdout line from the C++ process.
 * Expected: STATE:{…}  FINAL:{…}  INFO:{…}  ERROR:…
 * @param {string} line
 */
function parseLine(line) {
  if (line.startsWith("STATE:")) {
    try {
      stateQueue.push(JSON.parse(line.substring(6)));
    } catch (err) {
      console.error("Failed to parse STATE:", err.message, "| Raw:", line);
    }
  } else if (line.startsWith("FINAL:")) {
    try {
      simulationState = { ...simulationState, ...JSON.parse(line.substring(6)) };
      console.log("Simulation complete — final stats received.");
    } catch (err) {
      console.error("Failed to parse FINAL:", err.message);
    }
  } else if (line.startsWith("INFO:")) {
    try {
      console.log("Simulation info:", JSON.parse(line.substring(5)));
    } catch {
      /* non-fatal */
    }
  } else if (line.startsWith("ERROR:")) {
    console.error("C++ error:", line.substring(6));
  } else {
    console.warn("Unexpected output:", line);
  }
}

// ────────────────────────────────────────────────────────────────────────────
// Start & Cleanup:
// ────────────────
app.listen(PORT, () => {
  console.log(`Server running on: http://localhost:${PORT}`);
  console.log(`C++ binary: ${cppExecutable}`);
});

const gracefulShutdown = () => {
  console.log("\nShutting down Node.js server...");
  if (simulationProcess && !simulationProcess.killed) {
    simulationProcess.kill();
  }
  process.exit(0);
};

process.on("SIGINT", gracefulShutdown);
process.on("SIGTERM", gracefulShutdown);
