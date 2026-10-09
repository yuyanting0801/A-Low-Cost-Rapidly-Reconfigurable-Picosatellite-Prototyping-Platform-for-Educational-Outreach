/**
 * WebSocket client: connects to the server, routes incoming messages and reconnects
 * with exponential backoff + jitter.
 * Text frames carry sensor data or history; binary frames carry video (0x01) and photos (0x02).
 */
import { WS_URL, MAX_RECONNECT_DELAY, BASE_RECONNECT_DELAY } from './config.js';
import { processSensorData, processHistory, processVideoFrame, processPhoto } from './sensors.js';

let ws = null;
export let wsConnected = false;
let reconnectAttempts = 0;
let reconnectTimer = null;

export function connectWebSocket() {
    if (ws && (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING)) {
        return;
    }
    ws = new WebSocket(WS_URL);
    ws.binaryType = 'arraybuffer';

    ws.onopen = () => {
        wsConnected = true;
        reconnectAttempts = 0;
        document.getElementById('wsLed').classList.add('online');
        document.getElementById('wsLed').classList.remove('offline');
        document.getElementById('reconnectInfo').textContent = "";
        // Identity handshake: tells the server this connection is a frontend
        ws.send(JSON.stringify({ type: "frontend" }));
    };

    ws.onmessage = (event) => {
        if (typeof event.data === 'string') {
            try {
                const msg = JSON.parse(event.data);
                if (msg.type === "history") {
                    processHistory(msg.data);
                } else if (msg.type === "sensor") {
                    processSensorData(msg);
                } else if (msg.temperature !== undefined) {
                    processSensorData(msg);
                } else {
                    console.warn("Unknown text message type:", msg);
                }
            } catch (e) { console.error("JSON parse error", e); }
        } else if (event.data instanceof ArrayBuffer) {
            const data = new Uint8Array(event.data);
            if (data.length > 0) {
                // Frame type byte: 0x01 = video, 0x02 = photo
                if (data[0] === 0x01) processVideoFrame(data);
                else if (data[0] === 0x02) processPhoto(data);
            }
        }
    };

    ws.onclose = () => {
        wsConnected = false;
        document.getElementById('wsLed').classList.remove('online');
        document.getElementById('wsLed').classList.add('offline');
        const delay = Math.min(BASE_RECONNECT_DELAY * Math.pow(2, reconnectAttempts), MAX_RECONNECT_DELAY);
        const jitter = Math.random() * 1000;
        const totalDelay = delay + jitter;
        document.getElementById('reconnectInfo').textContent = `Reconnecting in ${Math.round(totalDelay / 1000)} seconds...`;
        if (reconnectTimer) clearTimeout(reconnectTimer);
        reconnectTimer = setTimeout(() => { connectWebSocket(); }, totalDelay);
        reconnectAttempts++;
    };

    ws.onerror = () => { ws.close(); };
}

export async function triggerCapture() {
    if (wsConnected) {
        ws.send(JSON.stringify({ type: "capture" }));
    } else {
        try {
            const res = await fetch('/api/capture', { method: 'POST' });
            if (!res.ok) alert("Capture failed");
        } catch (err) { alert("Error: " + err.message); }
    }
}
