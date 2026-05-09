const PORT = 8081;

export const BACKEND_CONFIG = {
  enabled: false,
  pollInterval: 350,
  baseUrl: `http://localhost:${PORT}`,
  endpoints: {
    state: "/api/state",
    config: "/api/config",
    pause: "/api/pause",
    reset: "/api/reset",
    start: "/api/start",
  },
};
