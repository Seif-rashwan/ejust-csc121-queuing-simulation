export const RING = {
  MAX_SLOTS: 100,
  canvas: null,
  ctx: null,

  init() {
    this.canvas = document.getElementById("ring-canvas");
    this.ctx = this.canvas.getContext("2d");
  },

  /**
   * Draws the circular array ring visualisation.
   * @param {number}   queueSize - Current number of customers in queue.
   * @param {number}   maxSlots  - Total capacity of the circular array.
   * @param {*}        _         - Reserved (unused, kept for call-site compat).
   * @param {Object[]} servers   - Array of server state objects.
   */
  draw(queueSize, maxSlots, _, servers) {
    const ctx = this.ctx;
    const canvas_w = this.canvas.width;
    const canvas_h = this.canvas.height;
    const center_x = canvas_w / 2;
    const center_y = canvas_h / 2;
    const radius = 210;

    let r_size;
    if (maxSlots <= 100) r_size = 10;
    else if (maxSlots <= 200) r_size = 7;
    else if (maxSlots <= 500) r_size = 5;
    else r_size = 3;

    ctx.clearRect(0, 0, canvas_w, canvas_h);

    // Glow ring track
    ctx.beginPath();
    ctx.arc(center_x, center_y, radius, 0, Math.PI * 2);
    ctx.strokeStyle = "rgba(100,120,255,0.15)";
    ctx.lineWidth = 24;
    ctx.stroke();

    ctx.beginPath();
    ctx.arc(center_x, center_y, radius, 0, Math.PI * 2);
    ctx.strokeStyle = "rgba(100,120,255,0.08)";
    ctx.lineWidth = 36;
    ctx.stroke();

    const numServers = servers ? servers.length : 1;
    const serverHues = [210, 45, 155, 0, 280, 180, 330, 90, 30, 260];

    for (let i = 0; i < maxSlots; i++) {
      const angle = (i / maxSlots) * Math.PI * 2 - Math.PI / 2;
      const x = center_x + radius * Math.cos(angle);
      const y = center_y + radius * Math.sin(angle);
      const isOccupied = i < queueSize;
      const isFront = isOccupied && i === 0;
      const isRear = isOccupied && i === queueSize - 1 && queueSize > 0;

      let ownerHue = 210;
      if (isOccupied && numServers > 1) {
        ownerHue = serverHues[(i % numServers) % serverHues.length];
      }

      if (isOccupied) {
        ctx.beginPath();
        ctx.arc(x, y, r_size + 5, 0, Math.PI * 2);
        ctx.fillStyle = isFront ? "rgba(80,230,120,0.25)" : `hsla(${ownerHue},80%,65%,0.18)`;
        ctx.fill();
      }

      ctx.beginPath();
      ctx.arc(x, y, r_size, 0, Math.PI * 2);

      if (isFront) {
        ctx.fillStyle = "#50e678";
        ctx.shadowColor = "#50e678";
        ctx.shadowBlur = 14;
      } else if (isRear) {
        ctx.fillStyle = "#f7b731";
        ctx.shadowColor = "#f7b731";
        ctx.shadowBlur = 14;
      } else if (isOccupied) {
        ctx.fillStyle = `hsl(${ownerHue},75%,62%)`;
        ctx.shadowColor = `hsl(${ownerHue},80%,55%)`;
        ctx.shadowBlur = 8;
      } else {
        ctx.fillStyle = "rgba(255,255,255,0.08)";
        ctx.shadowBlur = 0;
      }

      ctx.fill();
      ctx.shadowBlur = 0;

      if (i % 10 === 0) {
        ctx.fillStyle = "rgba(180,190,230,0.6)";
        ctx.font = "9px Inter, sans-serif";
        ctx.textAlign = "center";
        ctx.textBaseline = "middle";
        const lx = center_x + (radius + 22) * Math.cos(angle);
        const ly = center_y + (radius + 22) * Math.sin(angle);
        ctx.fillText(i, lx, ly);
      }
    }

    ctx.font = "bold 11px Inter, sans-serif";
    ctx.fillStyle = "rgba(150,170,255,0.7)";
    ctx.textAlign = "center";
    ctx.textBaseline = "middle";
    ctx.fillText("← FIFO", center_x, center_y - radius - 38);

    document.getElementById("ring-label-queue").textContent = queueSize;
    document.getElementById("ring-label-cap").textContent = `/ ${maxSlots} slots`;
  },
};
