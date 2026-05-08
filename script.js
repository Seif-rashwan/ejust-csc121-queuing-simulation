// =========================================================
// BACKEND CONFIG
// =========================================================

const BACKEND = {
  enabled: false,
  baseUrl: 'http://localhost:8080',

  pollInterval: 350,

  endpoints: {
    state: '/api/state',
    config: '/api/config',
    pause: '/api/pause',
    reset: '/api/reset'
  }
};

// =========================================================
// HELPERS
// =========================================================

function randomBetween(min, max){
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

// =========================================================
// LOCAL REALISTIC SIMULATION
// =========================================================

const local = {

  tick: 0,
  served: 0,
  totalWait: 0,

  queue: [],
  servers: [],

  nextId: 0,
  running: true,

  numServers: 4,

  arrivalMin: 2,
  arrivalMax: 6,

  serviceMin: 4,
  serviceMax: 10,

  nextArrivalTick: 0,

  reset(){

    this.tick = 0;
    this.served = 0;
    this.totalWait = 0;

    this.queue = [];
    this.nextId = 0;

    this.nextArrivalTick = randomBetween(
      this.arrivalMin,
      this.arrivalMax
    );

    this.servers = Array.from(
      { length: this.numServers },
      () => ({
        busy: false,
        remaining: 0,
        customerId: null
      })
    );
  },

  rebuildServers(){

    this.servers = Array.from(
      { length: this.numServers },
      () => ({
        busy: false,
        remaining: 0,
        customerId: null
      })
    );
  },

  tick_fn(){

    if(!this.running) return;

    this.tick++;

    if(this.tick >= this.nextArrivalTick){

      this.queue.push({
        id: this.nextId++,
        arrived: this.tick
      });

      this.nextArrivalTick =
        this.tick +
        randomBetween(
          this.arrivalMin,
          this.arrivalMax
        );
    }

    for(const s of this.servers){

      if(s.busy){

        s.remaining--;

        if(s.remaining <= 0){

          s.busy = false;
          s.customerId = null;

          this.served++;
        }
      }
    }

    for(const s of this.servers){

      if(!s.busy && this.queue.length > 0){

        const person = this.queue.shift();

        this.totalWait +=
          (this.tick - person.arrived);

        s.busy = true;

        s.remaining = randomBetween(
          this.serviceMin,
          this.serviceMax
        );

        s.customerId = person.id;
      }
    }
  },

  getState(){

    return {

      tick: this.tick,

      queueSize: this.queue.length,

      served: this.served,

      avgWait:
        this.served > 0
        ? this.totalWait / this.served
        : 0,

      nextArrival:
        Math.max(
          0,
          this.nextArrivalTick - this.tick
        ),

      running: this.running,

      servers:
        this.servers.map(s => ({
          busy: s.busy,
          remaining: s.remaining
        }))
    };
  }
};

// =========================================================
// BACKEND CALLS
// =========================================================

async function fetchState(){

  const res = await fetch(
    BACKEND.baseUrl +
    BACKEND.endpoints.state
  );

  if(!res.ok){
    throw new Error('State fetch failed');
  }

  return await res.json();
}

async function sendConfig(cfg){

  await fetch(
    BACKEND.baseUrl +
    BACKEND.endpoints.config,
    {
      method:'POST',
      headers:{
        'Content-Type':'application/json'
      },
      body: JSON.stringify(cfg)
    }
  );
}

async function sendPause(){

  await fetch(
    BACKEND.baseUrl +
    BACKEND.endpoints.pause,
    {
      method:'POST'
    }
  );
}

async function sendReset(){

  await fetch(
    BACKEND.baseUrl +
    BACKEND.endpoints.reset,
    {
      method:'POST'
    }
  );
}

// =========================================================
// RENDER UI
// =========================================================

function render(state){

  document.getElementById('s-tick')
    .textContent = state.tick;

  document.getElementById('s-queue')
    .textContent = state.queueSize;

  document.getElementById('s-served')
    .textContent = state.served;

  document.getElementById('s-wait')
    .textContent =
    state.avgWait.toFixed(1) + ' ticks';

  document.getElementById('s-next-arrival')
    .textContent =
    state.nextArrival + ' ticks';

  const qp =
    document.getElementById('queue-people');

  qp.innerHTML = '';

  const visible =
    Math.min(state.queueSize, 8);

  for(let i = 0; i < visible; i++){

    const person =
      document.createElement('span');

    person.className = 'person';
    person.textContent = '🧍';

    qp.appendChild(person);
  }

  if(state.queueSize > 8){

    const more =
      document.createElement('span');

    more.style.cssText =
      'font-size:11px;color:#8b92a5;align-self:center';

    more.textContent =
      '+' + (state.queueSize - 8);

    qp.appendChild(more);
  }

  const sr =
    document.getElementById('servers-row');

  sr.innerHTML = '';

  state.servers.forEach((s, i) => {

    const div =
      document.createElement('div');

    div.className = 'server';

    div.innerHTML = `
      <div class="server-label">
        S${i + 1}
      </div>

      <div class="server-icon ${s.busy ? 'busy' : 'idle'}">
        🧍
      </div>
    `;

    sr.appendChild(div);
  });
}

// =========================================================
// MAIN LOOP
// =========================================================

const statusBar =
  document.getElementById('status-bar');

let paused = false;

async function loop(){

  if(BACKEND.enabled){

    try{

      const state =
        await fetchState();

      render(state);

      paused = !state.running;

      const btn =
        document.getElementById('btn-pause');

      btn.textContent =
        paused ? 'Resume' : 'Pause';

      btn.classList.toggle(
        'active',
        !paused
      );

      statusBar.textContent =
        '● Connected to backend (real-time)';

      statusBar.className =
        'connected';

    }catch(e){

      statusBar.textContent =
        '● Backend unreachable — ' +
        e.message;

      statusBar.className =
        'error';
    }

  }else{

    local.tick_fn();

    render(local.getState());
  }
}

setInterval(
  loop,
  BACKEND.pollInterval
);

// =========================================================
// BUTTONS
// =========================================================

document.getElementById('btn-pause')
  .onclick = async function(){

    if(BACKEND.enabled){

      await sendPause();

    }else{

      local.running = !local.running;

      paused = !local.running;

      this.textContent =
        local.running
        ? 'Pause'
        : 'Resume';

      this.classList.toggle(
        'active',
        local.running
      );
    }
  };

document.getElementById('btn-reset')
  .onclick = async function(){

    if(BACKEND.enabled){

      await sendReset();

    }else{

      local.reset();

      render(local.getState());
    }
  };

// =========================================================
// SLIDERS
// =========================================================

function updateArrivalLabel(){

  document.getElementById('v-arrival')
    .textContent =
    local.arrivalMin +
    '-' +
    local.arrivalMax;
}

function updateServiceLabel(){

  document.getElementById('v-service')
    .textContent =
    local.serviceMin +
    '-' +
    local.serviceMax;
}

const serverSlider =
  document.getElementById('r-servers');

serverSlider.oninput = async () => {

  local.numServers =
    +serverSlider.value;

  document.getElementById('v-servers')
    .textContent =
    serverSlider.value;

  local.rebuildServers();

  if(BACKEND.enabled){

    await sendConfig({
      servers: local.numServers
    });
  }
};

document.getElementById('r-arrival-min')
  .oninput = () => {

    local.arrivalMin =
      +document.getElementById(
        'r-arrival-min'
      ).value;

    if(local.arrivalMin >= local.arrivalMax){

      local.arrivalMax =
        local.arrivalMin + 1;

      document.getElementById(
        'r-arrival-max'
      ).value =
      local.arrivalMax;
    }

    updateArrivalLabel();
  };

document.getElementById('r-arrival-max')
  .oninput = () => {

    local.arrivalMax =
      +document.getElementById(
        'r-arrival-max'
      ).value;

    if(local.arrivalMax <= local.arrivalMin){

      local.arrivalMin =
        local.arrivalMax - 1;

      document.getElementById(
        'r-arrival-min'
      ).value =
      local.arrivalMin;
    }

    updateArrivalLabel();
  };

document.getElementById('r-service-min')
  .oninput = () => {

    local.serviceMin =
      +document.getElementById(
        'r-service-min'
      ).value;

    if(local.serviceMin >= local.serviceMax){

      local.serviceMax =
        local.serviceMin + 1;

      document.getElementById(
        'r-service-max'
      ).value =
      local.serviceMax;
    }

    updateServiceLabel();
  };

document.getElementById('r-service-max')
  .oninput = () => {

    local.serviceMax =
      +document.getElementById(
        'r-service-max'
      ).value;

    if(local.serviceMax <= local.serviceMin){

      local.serviceMin =
        local.serviceMax - 1;

      document.getElementById(
        'r-service-min'
      ).value =
      local.serviceMin;
    }

    updateServiceLabel();
  };

// =========================================================
// INIT
// =========================================================

local.reset();

render(local.getState());