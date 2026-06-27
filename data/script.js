let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

window.addEventListener('load', () => {
    initWebSocket();
});

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen = () => console.log("WS connecté");
    websocket.onclose = () => setTimeout(initWebSocket, 2000);
    websocket.onmessage = onMessage;
}

function onMessage(event) {
    const data = JSON.parse(event.data);

    // --- Logs ---
    if (data.log) {
        addLog(data.log.type, data.log.msg);
        return; // rien d'autre à traiter
    }

    // --- CAN Monitor ---
    if (data.can_frame) {
        addCanFrame(data.can_frame);
        return;
    }

    // --- États simples ---
    if (data.wifi_on !== undefined)
        document.getElementById("wifi_on").checked = data.wifi_on;

    if (data.exploration_on !== undefined)
        document.getElementById("exploration_on").checked = data.exploration_on;

    if (data.track_profile !== undefined)
        document.getElementById("track_profile").value = data.track_profile;

    if (data.mode_test !== undefined)
        document.getElementById("mode_test").checked = data.mode_test;

    // --- État CAN ---
    if (data.can_ok !== undefined) {
        const canBox = document.getElementById("can_status");
        if (data.can_ok) {
            canBox.style.color = "#22c55e";
            canBox.innerText = `CAN OK (${data.can_last_ms} ms)`;
        } else {
            canBox.style.color = "#ef4444";
            canBox.innerText = "CAN OFFLINE";
        }
    }

    // --- Canton Controller ---
    if (Array.isArray(data.ccs)) {
        let html = "";
        data.ccs.forEach(s => {
            html += `
            <div class="cc-item">
                <div>
                    <div class="cc-id">CC ${s.id}</div>
                    <div class="cc-lastseen">Vu il y a ${s.lastSeen} ms</div>
                </div>
                <div class="${s.online ? 'cc-online' : 'cc-offline'}">
                    ${s.online ? 'ONLINE' : 'OFFLINE'}
                </div>
            </div>`;
        });
        document.getElementById("cc_list").innerHTML = html;
    }

    // --- STOP ---
    if (data.stop_state !== undefined) {
        const box = document.getElementById("stop_state");
        if (data.stop_state === 1) {
            box.innerText = "État : STOP actif";
            box.className = "status stop-active";
        } else {
            box.innerText = "État : fonctionnement normal";
            box.className = "status stop-clear";
        }
    }

    // --- Sauvegarde ---
    if (data.save_state !== undefined) {
        const box = document.getElementById("save_state");
        box.className = "status";

        if (data.save_state === 0) {
            box.innerText = "Sauvegarde : en attente";
        }
        if (data.save_state === 1) {
            box.innerText = "Sauvegarde : OK";
            box.className = "status state-ok";
        }
        if (data.save_state === 2) {
            box.innerText = "Sauvegarde : erreur";
            box.className = "status state-error";
        }
    }

    // --- Redémarrage ---
    if (data.restart_state !== undefined) {
        const box = document.getElementById("restart_state");
        box.className = "status";

        if (data.restart_state === 0) {
            box.innerText = "Redémarrage : en attente";
        }
        if (data.restart_state === 1) {
            box.innerText = "Redémarrage : OK";
            box.className = "status state-ok";
        }
        if (data.restart_state === 2) {
            box.innerText = "Redémarrage : erreur";
            box.className = "status state-error";
        }
    }
}

/* ============================================================
   COMMANDES WEB
   ============================================================ */

function wifi_on(el) {
    websocket.send(JSON.stringify({ wifi_on: el.checked }));
}

function exploration_on(el) {
    websocket.send(JSON.stringify({ exploration_on: el.checked }));
}

function mode_test(el) {
    websocket.send(JSON.stringify({ mode_test: el.checked }));
}

function set_profile() {
    const v = Number(document.getElementById("track_profile").value);
    websocket.send(JSON.stringify({ set_profile: true, value: v }));
}

/* ============================================================
   SAUVEGARDE
   ============================================================ */
function save() {
    websocket.send(JSON.stringify({ save: true }));
    const box = document.getElementById("save_state");
    box.innerText = "Sauvegarde : en cours…";
    box.className = "status state-working";
}

/* ============================================================
   REDÉMARRAGE
   ============================================================ */
function restartEsp() {
    websocket.send(JSON.stringify({ restartEsp: true }));
    const box = document.getElementById("restart_state");
    box.innerText = "Redémarrage : en cours…";
    box.className = "status state-working";
}

/* ============================================================
   STOP GLOBAL
   ============================================================ */
function send_stop() {
    websocket.send(JSON.stringify({ stop: true }));
    const box = document.getElementById("stop_state");
    box.innerText = "État : STOP actif";
    box.className = "status stop-active";
}

/* ============================================================
   CLEAR STOP GLOBAL
   ============================================================ */
function send_clear_stop() {
    websocket.send(JSON.stringify({ clear_stop: true }));
    const box = document.getElementById("stop_state");
    box.innerText = "État : fonctionnement normal";
    box.className = "status stop-clear";
}

/* ============================================================
   LOGS
   ============================================================ */

function addLog(type, msg) {
    const box = document.getElementById("log_console");
    const line = document.createElement("div");
    line.className = "log-" + type;
    line.textContent = msg;
    box.appendChild(line);
    box.scrollTop = box.scrollHeight;
}

/* ============================================================
   CAN MONITOR
   ============================================================ */

function addCanFrame(frame) {
    const box = document.getElementById("can_monitor");
    const div = document.createElement("div");
    div.className = "can-frame can-" + frame.type;

    div.textContent =
        `[${frame.time} ms] ${frame.type.toUpperCase()} ID=0x${frame.id.toString(16)} DLC=${frame.dlc} DATA=${frame.data}`;

    box.appendChild(div);
    box.scrollTop = box.scrollHeight;
}
