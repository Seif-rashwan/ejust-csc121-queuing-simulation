/**
 * @file ring.js
 * Canvas renderer for the circular-array queue visualization.
 */

/** Circular queue canvas controller. */
export const RING = {
  MAX_SLOTS: 100,
  canvas: null,
  ctx: null,

  /**
   * Initializes the canvas and drawing context.
   */
  init() {
    this.canvas = document.getElementById("ring-canvas");
    this.ctx = this.canvas.getContext("2d");
  },

  /**
   * Reads a CSS variable from the active theme.
   * @param {string} name CSS variable name.
   * @param {string} fallback Fallback color.
   * @returns {string} Resolved color.
   */
  color(name, fallback) {
    const value = getComputedStyle(document.body).getPropertyValue(name).trim();
    return value || fallback;
  },

  /**
   * Draws the circular array ring visualisation.
   * @param {number} queueSize Current number of customers in queue.
   * @param {number} maxSlots Total capacity of the circular array.
   * @param {*} _ Reserved for call-site compatibility.
   * @param {Object[]} servers Array of server state objects.
   */
  draw(queueSize, maxSlots, _, servers) {
    const ctx = this.ctx;
    const canvasW = this.canvas.width;
    const canvasH = this.canvas.height;
    const centerX = canvasW / 2;
    const centerY = canvasH / 2;
    const radius = 210;

    let dotSize;
    if (maxSlots <= 100) dotSize = 10;
    else if (maxSlots <= 200) dotSize = 7;
    else if (maxSlots <= 500) dotSize = 5;
    else dotSize = 3;

    const accent = this.color("--accent", "#4f7cff");
    const success = this.color("--success", "#14b88a");
    const warning = this.color("--warning", "#f59e0b");
    const empty = this.color("--ring-empty", "rgba(255,255,255,0.08)");
    const label = this.color("--ring-label", "rgba(150,170,255,0.7)");
    const track = this.color("--ring-track", "rgba(100,120,255,0.15)");
    const trackSoft = this.color("--ring-track-soft", "rgba(100,120,255,0.08)");

    ctx.clearRect(0, 0, canvasW, canvasH);

    ctx.beginPath();
    ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
    ctx.strokeStyle = track;
    ctx.lineWidth = 24;
    ctx.stroke();

    ctx.beginPath();
    ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
    ctx.strokeStyle = trackSoft;
    ctx.lineWidth = 36;
    ctx.stroke();

    const numServers = servers ? servers.length : 1;
    const serverHues = [220, 38, 164, 0, 260, 188, 326, 156, 22, 207];

    for (let i = 0; i < maxSlots; i++) {
      const angle = (i / maxSlots) * Math.PI * 2 - Math.PI / 2;
      const x = centerX + radius * Math.cos(angle);
      const y = centerY + radius * Math.sin(angle);
      const isOccupied = i < queueSize;
      const isFront = isOccupied && i === 0;
      const isRear = isOccupied && i === queueSize - 1 && queueSize > 0;

      let ownerHue = 220;
      if (isOccupied && numServers > 1) {
        ownerHue = serverHues[(i % numServers) % serverHues.length];
      }

      if (isOccupied) {
        ctx.beginPath();
        ctx.arc(x, y, dotSize + 5, 0, Math.PI * 2);
        ctx.fillStyle = isFront ? "rgba(20,184,138,0.22)" : `hsla(${ownerHue},80%,60%,0.16)`;
        ctx.fill();
      }

      ctx.beginPath();
      ctx.arc(x, y, dotSize, 0, Math.PI * 2);

      if (isFront) {
        ctx.fillStyle = success;
        ctx.shadowColor = success;
        ctx.shadowBlur = 14;
      } else if (isRear) {
        ctx.fillStyle = warning;
        ctx.shadowColor = warning;
        ctx.shadowBlur = 14;
      } else if (isOccupied) {
        ctx.fillStyle = numServers > 1 ? `hsl(${ownerHue},72%,58%)` : accent;
        ctx.shadowColor = numServers > 1 ? `hsl(${ownerHue},72%,48%)` : accent;
        ctx.shadowBlur = 8;
      } else {
        ctx.fillStyle = empty;
        ctx.shadowBlur = 0;
      }

      ctx.fill();
      ctx.shadowBlur = 0;

      if (i % 10 === 0) {
        ctx.fillStyle = label;
        ctx.font = "9px Segoe UI, sans-serif";
        ctx.textAlign = "center";
        ctx.textBaseline = "middle";
        const labelX = centerX + (radius + 22) * Math.cos(angle);
        const labelY = centerY + (radius + 22) * Math.sin(angle);
        ctx.fillText(i, labelX, labelY);
      }
    }

    ctx.font = "bold 11px Segoe UI, sans-serif";
    ctx.fillStyle = label;
    ctx.textAlign = "center";
    ctx.textBaseline = "middle";
    ctx.fillText("FIFO", centerX, centerY - radius - 38);

    document.getElementById("ring-label-queue").textContent = queueSize;
    document.getElementById("ring-label-cap").textContent = `/ ${maxSlots} slots`;
  },
};
