const state = {
  nextReminderId: null,
  remindersVisible: false
};

function updateClock() {
  const now = new Date();
  document.getElementById('clock').textContent = now.toLocaleTimeString('zh-CN', { hour: '2-digit', minute: '2-digit' });
  document.getElementById('date').textContent = now.toLocaleDateString('zh-CN', { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' });
}

async function jsonFetch(url, options) {
  const res = await fetch(url, options);
  return res.json();
}

async function refreshStatus() {
  const status = await jsonFetch('/api/device/status');
  const device = status.device;
  document.getElementById('deviceStatus').textContent = `${device.device_name} · ${device.state} · ${device.presence ? '有人' : '未检测到人'}`;
  document.getElementById('unreadCount').textContent = `${status.unread_messages} 条`;
  if (status.next_reminder) {
    state.nextReminderId = status.next_reminder.id;
    document.getElementById('nextReminder').textContent = `${status.next_reminder.time} ${status.next_reminder.title}`;
  } else {
    state.nextReminderId = null;
    document.getElementById('nextReminder').textContent = '暂无提醒';
  }
}

async function refreshReminders() {
  const reminders = await jsonFetch('/api/reminders');
  document.getElementById('reminderList').innerHTML = reminders.map(r =>
    `<div class="item"><span>${r.time} ${r.title}</span><strong>${r.type}</strong></div>`
  ).join('');
}

document.getElementById('ackBtn').addEventListener('click', async () => {
  if (state.nextReminderId) {
    await fetch(`/api/reminders/ack?id=${state.nextReminderId}`, { method: 'POST' });
  }
  await refreshStatus();
});

document.getElementById('playMsgBtn').addEventListener('click', async () => {
  await fetch('/api/messages/play-next', { method: 'POST' });
  await refreshStatus();
});

document.getElementById('callBtn').addEventListener('click', async () => {
  await fetch('/api/call/primary', { method: 'POST' });
  await refreshStatus();
});

document.getElementById('showRemindersBtn').addEventListener('click', async () => {
  state.remindersVisible = !state.remindersVisible;
  document.getElementById('reminderPanel').hidden = !state.remindersVisible;
  if (state.remindersVisible) await refreshReminders();
});

updateClock();
refreshStatus();
setInterval(updateClock, 1000);
setInterval(refreshStatus, 5000);
