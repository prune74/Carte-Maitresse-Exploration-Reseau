const gateway = 'ws://' + window.location.hostname + '/ws';
let websocket;

function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage;
}

function onOpen(event) {
  console.log('Connection opened');
  document.getElementById('messages').innerHTML = "Connected";
}

function onClose(event) {
  console.log('Connection closed');
  document.getElementById('messages').innerHTML = "Connection closed";
  setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
  console.log("WS:", event.data);

  let data = JSON.parse(event.data);

  // --- États globaux ---
  document.getElementById('wifi_on').checked      = data.wifi_on;
  document.getElementById('discovery_on').checked = data.discovery_on;

  // --- État CAN ---
  let canLed = document.getElementById('can_status');
  if (data.can_ok) {
    canLed.style.backgroundColor = "green";
    canLed.innerHTML = "CAN OK (" + data.can_last_ms + " ms)";
  } else {
    canLed.style.backgroundColor = "red";
    canLed.innerHTML = "CAN OFF (" + data.can_last_ms + " ms)";
  }

  // --- Liste des satellites ---
  let satDiv = document.getElementById('sat_list');
  satDiv.innerHTML = ""; // reset

  data.sats.forEach(s => {
    let color = s.online ? "green" : "red";
    satDiv.innerHTML += `
      <div style="padding:4px; margin:3px; border:1px solid #ccc;">
        <b>Satellite ${s.id}</b> :
        <span style="color:${color}; font-weight:bold;">
          ${s.online ? "ONLINE" : "OFFLINE"}
        </span>
        <br>
        lastSeen = ${s.lastSeen} ms
      </div>
    `;
  });
}

window.addEventListener('load', () => {
  initWebSocket();
});

// --- Commandes envoyées au serveur ---
function wifi_on(obj) {
  websocket.send(JSON.stringify({ wifi_on: obj.checked }));
}

function discovery_on(obj) {
  websocket.send(JSON.stringify({ discovery_on: obj.checked }));
}

function restartEsp() {
  websocket.send(JSON.stringify({ restartEsp: true }));
}

function save() {
  websocket.send(JSON.stringify({ save: true }));
}
