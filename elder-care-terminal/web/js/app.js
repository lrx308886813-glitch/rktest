(async function () {
  const el = document.getElementById('health');
  if (!el) return;
  const res = await fetch('/api/health');
  el.textContent = `health: ${await res.text()}`;
})();
