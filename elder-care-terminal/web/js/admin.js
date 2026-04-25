async function jsonFetch(url, options) {
  const res = await fetch(url, options);
  return res.json();
}

async function postJson(url, data) {
  return jsonFetch(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  });
}

async function refreshAdmin() {
  const [status, reminders, contacts, logs] = await Promise.all([
    jsonFetch('/api/device/status'),
    jsonFetch('/api/reminders'),
    jsonFetch('/api/contacts'),
    jsonFetch('/api/logs/recent')
  ]);
  document.getElementById('adminStatus').textContent =
    `${status.device.device_name} · ${status.device.state} · 未读留言 ${status.unread_messages}`;
  document.getElementById('adminReminders').innerHTML = reminders.map(r =>
    `<div class="item"><span>${r.time} ${r.title}</span><strong>${r.type}</strong></div>`
  ).join('');
  document.getElementById('contacts').innerHTML = contacts.map(c =>
    `<div class="item"><span>${c.name} ${c.relation}</span><strong>${c.primary ? '主要' : c.phone}</strong></div>`
  ).join('');
  document.getElementById('logs').textContent = logs.join('\n');
}

document.getElementById('reminderForm').addEventListener('submit', async (event) => {
  event.preventDefault();
  const form = new FormData(event.target);
  await postJson('/api/reminders', {
    title: form.get('title'),
    type: form.get('type'),
    time: form.get('time'),
    enabled: true
  });
  event.target.reset();
  await refreshAdmin();
});

document.getElementById('contactForm').addEventListener('submit', async (event) => {
  event.preventDefault();
  const form = new FormData(event.target);
  await postJson('/api/contacts', {
    name: form.get('name'),
    phone: form.get('phone'),
    relation: form.get('relation'),
    primary: form.get('primary') === 'on'
  });
  event.target.reset();
  await refreshAdmin();
});

document.getElementById('messageForm').addEventListener('submit', async (event) => {
  event.preventDefault();
  const form = new FormData(event.target);
  await postJson('/api/messages', {
    sender_name: form.get('sender_name'),
    content: form.get('content')
  });
  event.target.reset();
  await refreshAdmin();
});

refreshAdmin();
setInterval(refreshAdmin, 6000);
