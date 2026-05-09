/**
 * Generates a random integer between min and max (inclusive).
 * @param {number} min
 * @param {number} max
 * @returns {number}
 */
export function randomBetween(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

/**
 * Sets the textContent of a DOM element by ID. No-ops if element not found.
 * @param {string} id
 * @param {string|number} val
 */
export function setText(id, val) {
  const el = document.getElementById(id);
  if (el) el.textContent = val;
}
