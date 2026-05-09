const express = require('express');
const { spawn } = require('child_process');
const path = require('path');
const cors = require('cors');

const app = express();
const PORT = 8081;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static('../frontend'));

// Simulation state
let simulationProcess = null;
let stateQueue = [];          // buffered STATE snapshots — served one per poll
let simulationState = {
    tick: 0,
    queueSize: 0,
    served: 0,
    avgWait: 0,
    nextArrival: 0,
    running: false,
    turnedAway: 0,
    peakQueue: 0,
    lastEvent: '',
    lastEventCustomer: -1,
    totalCustomers: 100,
    arrived: 0,
    servers: []
};

// Configuration
let config = {
    servers: 4,
    arrivalMin: 1,
    arrivalMax: 3,
    serviceMin: 8,
    serviceMax: 15,
    maxCustomers: 100  // = total customers who will arrive
};

// ── API Routes ────────────────────────────────────────────────────────────────

// /api/state: pop one queued state per poll (step-by-step replay)
app.get('/api/state', (req, res) => {
    if (stateQueue.length > 0) {
        const next = stateQueue.shift();
        simulationState = { ...simulationState, ...next };
    }
    res.json(simulationState);
});

app.post('/api/config', (req, res) => {
    config = { ...config, ...req.body };
    console.log('Configuration updated:', config);
    res.json({ success: true });
});

app.post('/api/start', (req, res) => {
    // Kill any previously running simulation
    if (simulationProcess && !simulationProcess.killed) {
        simulationProcess.kill();
        simulationProcess = null;
    }

    const cppExecutable = path.join(__dirname, 'build', 'simulation.exe');
    // argv[1] = safety cap (very large), argv[5] = total customers to arrive
    const safetyTime = config.maxCustomers * config.arrivalMin * 20; // generous cap
    const args = [
        safetyTime.toString(),
        config.servers.toString(),
        config.serviceMin.toString(),
        config.arrivalMin.toString(),
        config.maxCustomers.toString()
    ];
    stateQueue = [];  // clear any previous run

    simulationProcess = spawn(cppExecutable, args);

    // ── Bug fix: buffer incomplete lines across data chunks ──────────────────
    let lineBuffer = '';

    simulationProcess.stdout.on('data', (data) => {
        lineBuffer += data.toString();
        const lines = lineBuffer.split('\n');
        // Keep the last (possibly incomplete) fragment in the buffer
        lineBuffer = lines.pop();
        lines.forEach(line => {
            line = line.trim();
            if (line.length > 0) parseLine(line);
        });
    });

    simulationProcess.stderr.on('data', (data) => {
        console.error('Simulation error:', data.toString());
    });

    simulationProcess.on('close', (code) => {
        console.log(`Simulation process exited with code ${code}`);
        // Flush any remaining buffered content
        if (lineBuffer.trim().length > 0) parseLine(lineBuffer.trim());
        lineBuffer = '';
        simulationState.running = false;
    });

    simulationState.running = true;
    res.json({ success: true });
});

// ── Bug fix: pause actually signals the C++ process ──────────────────────────
app.post('/api/pause', (req, res) => {
    if (!simulationProcess || simulationProcess.killed) {
        return res.json({ success: false, message: 'No simulation running' });
    }

    // Toggle: kill the process on pause (it will need to restart on resume).
    // For a simpler UX the frontend should call /api/start again to resume.
    simulationProcess.kill();
    simulationProcess = null;
    simulationState.running = false;

    res.json({ success: true, running: false });
});

app.post('/api/reset', (req, res) => {
    if (simulationProcess && !simulationProcess.killed) {
        simulationProcess.kill();
        simulationProcess = null;
    }

    stateQueue = [];
    simulationState = {
        tick: 0,
        queueSize: 0,
        served: 0,
        avgWait: 0,
        nextArrival: 0,
        running: false,
        turnedAway: 0,
        peakQueue: 0,
        lastEvent: '',
        lastEventCustomer: -1,
        totalCustomers: config.maxCustomers,
        arrived: 0,
        servers: Array.from({ length: config.servers }, () => ({
            busy: false,
            remaining: 0,
            customerId: -1
        }))
    };

    res.json({ success: true });
});

// ── Output parser ─────────────────────────────────────────────────────────────

/**
 * Parse a single complete line from the C++ process.
 * All meaningful output uses a "PREFIX:{json}" or "PREFIX:{}" format.
 */
function parseLine(line) {
    if (line.startsWith('STATE:')) {
        try {
            const newState = JSON.parse(line.substring(6));
            // Push to queue — /api/state pops one per poll for step-by-step replay
            stateQueue.push(newState);
        } catch (e) {
            console.error('Error parsing STATE line:', e.message, '| Line:', line);
        }

    } else if (line.startsWith('FINAL:')) {
        try {
            const finalStats = JSON.parse(line.substring(6));
            console.log('Final statistics:', finalStats);
            // Merge final stats into simulationState so the UI can read them
            simulationState = { ...simulationState, ...finalStats };
        } catch (e) {
            console.error('Error parsing FINAL line:', e.message);
        }

    } else if (line.startsWith('INFO:')) {
        try {
            const info = JSON.parse(line.substring(5));
            console.log('Simulation info:', info);
        } catch (e) { /* non-fatal */ }

    } else if (line.startsWith('ERROR:')) {
        console.error('C++ simulation error:', line.substring(6));

    } else if (line.length > 0) {
        // Log unexpected output instead of silently ignoring it
        console.warn('Unexpected output from simulation:', line);
    }
}

// ── Initialize servers array ──────────────────────────────────────────────────
simulationState.servers = Array.from({ length: config.servers }, () => ({
    busy: false,
    remaining: 0,
    customerId: -1
}));

app.listen(PORT, () => {
    console.log(`Queuing system server running on http://localhost:${PORT}`);
});
