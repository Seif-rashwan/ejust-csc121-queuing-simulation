import { fileURLToPath } from "node:url";
import { spawn } from "node:child_process";
import express from "express";
import path from "node:path";
import cors from "cors";
import fs from "node:fs";

const app = express();
const PORT = 8081;
const __dirname = path.dirname(fileURLToPath(import.meta.url));

app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, "..", "frontend")));

// ────────────────────────────────────────────────────────────────────────────────
// Runtime State:
// ──────────────
let simulationProcess = null;
let stateQueue = [];
let paused = false;
let starting = false;

const MAX_QUEUE_SIZE = 5000;
const exeName = process.platform === "win32" ? "simulation.exe" : "simulation";
const cppExecutable = path.join(__dirname, "..", "build", "bin", exeName);

let config = {
  servers: 4,
  arrivalMin: 1,
  arrivalMax: 3,
  serviceMin: 8,
  serviceMax: 15,
  maxCustomers: 100,
};

let simulationState = _blankState();

function _blankState() {
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
    servers: _blankServers(),
  };
}

function _blankServers() {
  return Array.from({ length: config.servers }, () => ({
    busy: false,
    remaining: 0,
    customerId: -1,
  }));
}

// ────────────────────────────────────────────────────────────────────────────────
// Routes:
// ───────
app.get("/api/state", (_, res) => {
  if (!paused && stateQueue.length > 0) {
    simulationState = { ...simulationState, ...stateQueue.shift() };
  }
  res.json(simulationState);
});

app.post("/api/config", (req, res) => {
  config = { ...config, ...req.body };
  console.log("Config updated:", config);
  res.json({ success: true });
});

app.post("/api/start", (_, res) => {

  console.log("Looking for executable at:", cppExecutable); // add this

  if (starting) {
    res.status(429).json({ success: false, message: "Already starting." });
    return;
  }

  if (!fs.existsSync(cppExecutable)) {
    res.status(500).json({
      success: false,
      message: `Executable not found at ${cppExecutable}. Run 'make' first.`,
    });

    return;
  }

  if (simulationProcess && !simulationProcess.killed) {
    simulationProcess.kill();
    simulationProcess = null;
  }

  const transactionTime = Math.round((config.serviceMin + config.serviceMax) / 2);
  const timeBetweenArrivals = Math.round((config.arrivalMin + config.arrivalMax) / 2);
  const safetyTime = config.maxCustomers * Math.max(config.arrivalMin, config.serviceMax) * 10;

  const args = [
    safetyTime.toString(),
    config.servers.toString(),
    transactionTime.toString(),
    timeBetweenArrivals.toString(),
    config.maxCustomers.toString(),
  ];

  stateQueue = [];
  paused = false;
  starting = true;

  simulationProcess = spawn(cppExecutable, args);
  let lineBuffer = "";

  simulationProcess.stdout.on("data", (data) => {
    lineBuffer += data.toString();
    const lines = lineBuffer.split("\n");
    lineBuffer = lines.pop() ?? "";

    lines.forEach((line) => {
      line = line.trim();
      if (line.length === 0) return;
      if (stateQueue.length < MAX_QUEUE_SIZE) parseLine(line);
    });
  });

  simulationProcess.stderr.on("data", (data) => {
    console.error("Simulation stderr:", data.toString());
  });

  simulationProcess.on("error", (err) => {
    console.error("Failed to spawn simulation:", err.message);
    simulationState.running = false;
    starting = false;
  });

  simulationProcess.on("close", (code) => {
    console.log(`Simulation exited with code ${code}`);
    if (lineBuffer.trim().length > 0) parseLine(lineBuffer.trim());
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
  simulationState = _blankState();

  res.json({ success: true });
});

// ────────────────────────────────────────────────────────────────────────────────
// Output parser:
// ──────────────
/**
 * Parses a single complete line from the C++ process stdout.
 * Expected formats: STATE:{...}  FINAL:{...}  INFO:{...}  ERROR:...
 * @param {string} line
 */
function parseLine(line) {
  if (line.startsWith("STATE:")) {
    try {
      stateQueue.push(JSON.parse(line.substring(6)));
    } catch (err) {
      console.error("Failed to parse STATE line:", err.message, "| Raw:", line);
    }
  } else if (line.startsWith("FINAL:")) {
    try {
      simulationState = { ...simulationState, ...JSON.parse(line.substring(6)) };
    } catch (err) {
      console.error("Failed to parse FINAL line:", err.message);
    }
  } else if (line.startsWith("INFO:")) {
    try {
      console.log("Simulation info:", JSON.parse(line.substring(5)));
    } catch {
      /* Non-fatal */
    }
  } else if (line.startsWith("ERROR:")) {
    console.error("C++ error:", line.substring(6));
  } else {
    console.warn("Unexpected simulation output:", line);
  }
}

// ────────────────────────────────────────────────────────────────────────────────
// Start
// ──────
app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});
