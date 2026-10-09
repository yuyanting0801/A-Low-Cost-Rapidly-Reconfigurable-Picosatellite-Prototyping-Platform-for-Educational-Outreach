/**
 * Main entry module: initialises charts, the 3D scene and the WebSocket,
 * polls device status, and drives the clock and button bindings.
 */
import { initCharts } from './charts.js';
import { initThree } from './three3d.js';
import { connectWebSocket, triggerCapture } from './websocket.js';
import { SENDER_IP, RECEIVER_IP } from './config.js';

async function checkDeviceStatus(url, ledElement) {
    try {
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), 2000);
        const response = await fetch(url, { signal: controller.signal, method: 'HEAD' });
        clearTimeout(timeoutId);
        if (response.ok) {
            ledElement.classList.add('online');
            ledElement.classList.remove('offline');
        } else throw new Error();
    } catch {
        ledElement.classList.remove('online');
        ledElement.classList.add('offline');
    }
}

async function checkAllStatus() {
    await checkDeviceStatus(`http://${SENDER_IP}/sensor`, document.getElementById('senderLed'));
    await checkDeviceStatus(`http://${RECEIVER_IP}/latest_photo`, document.getElementById('receiverLed'));
}

function updateDateTime() {
    document.getElementById('datetime').textContent = new Date().toLocaleString('en-US', { hour12: false });
}

window.onload = () => {
    initCharts();
    initThree();
    connectWebSocket();
    document.getElementById('captureBtn').onclick = triggerCapture;
    setInterval(checkAllStatus, 5000);
    checkAllStatus();
    setInterval(updateDateTime, 1000);
    updateDateTime();
};
