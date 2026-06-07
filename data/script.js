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

    // --- États simples ---
    document.getElementById("wifi_on").checked = data.wifi_on;
    document.getElementById("discovery_on").checked = data.discovery_on;
    document.getElementById("track_profile").value = data.track_profile;

    // 🔥 Nouveau : Mode Test
    if (data.mode_test !== undefined) {
        document.getElementById("mode_test").checked = data.mode_test;
    }

    // --- État CAN ---
    const canBox = document.getElementById("can_status");
    if (data.can_ok) {
        canBox.style.color = "#22c55e";
        canBox.innerText = `CAN OK (${data.can_last_ms} ms)`;
    } else {
        canBox.style.color = "#ef4444";
        canBox.innerText = "CAN OFFLINE";
    }

    // --- Liste des satellites ---
    let html = "";
    data.sats.forEach(s => {
        html += `<div class="sat-item">
                    <b>ID ${s.id}</b> —
                    <span class="${s.online ? 'sat-online' : 'sat-offline'}">
                        ${s.online ? 'ONLINE' : 'OFFLINE'}
                    </span>
                 </div>`;
    });
    document.getElementById("sat_list").innerHTML = html;

    // --- 🟥🟩 État STOP / CLEAR STOP ---
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

    // --- 💾 État SAUVEGARDE ---
    if (data.save_state !== undefined) {
        const box = document.getElementById("save_state");

        if (data.save_state === 0) {
            box.innerText = "Sauvegarde : en attente";
            box.className = "status";
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

    // --- 🔄 État REDÉMARRAGE ---
    if (data.restart_state !== undefined) {
        const box = document.getElementById("restart_state");

        if (data.restart_state === 0) {
            box.innerText = "Redémarrage : en attente";
            box.className = "status";
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

function discovery_on(el) {
    websocket.send(JSON.stringify({ discovery_on: el.checked }));
}

// 🔥 Nouveau : Mode Test
function mode_test(el) {
    websocket.send(JSON.stringify({ mode_test: el.checked }));
}

function set_profile() {
    const v = Number(document.getElementById("track_profile").value);
    websocket.send(JSON.stringify({ set_profile: true, value: v }));
}

/* ============================================================
   💾 SAUVEGARDE
   ============================================================ */
function save() {
    websocket.send(JSON.stringify({ save: true }));

    const box = document.getElementById("save_state");
    box.innerText = "Sauvegarde : en cours…";
    box.className = "status state-working";

    console.log("Sauvegarde envoyée");
}

/* ============================================================
   🔄 REDÉMARRAGE
   ============================================================ */
function restartEsp() {
    websocket.send(JSON.stringify({ restartEsp: true }));

    const box = document.getElementById("restart_state");
    box.innerText = "Redémarrage : en cours…";
    box.className = "status state-working";

    console.log("Redémarrage envoyé");
}

/* ============================================================
   🟥 STOP GLOBAL (0x201)
   ============================================================ */
function send_stop() {
    websocket.send(JSON.stringify({ stop: true }));

    const box = document.getElementById("stop_state");
    box.innerText = "État : STOP actif";
    box.className = "status stop-active";

    console.log("STOP global envoyé");
}

/* ============================================================
   🟩 CLEAR STOP GLOBAL (0x202)
   ============================================================ */
function send_clear_stop() {
    websocket.send(JSON.stringify({ clear_stop: true }));

    const box = document.getElementById("stop_state");
    box.innerText = "État : fonctionnement normal";
    box.className = "status stop-clear";

    console.log("CLEAR STOP envoyé");
}
