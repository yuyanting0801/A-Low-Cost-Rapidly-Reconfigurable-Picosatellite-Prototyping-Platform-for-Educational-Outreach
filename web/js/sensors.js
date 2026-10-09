/**
 * Sensor data handling: validation, panel updates, and display of binary
 * video frames and photos.
 */
import { toSeconds, MAX_HISTORY } from './config.js';
import { updateArrow } from './three3d.js';
import { addDataPoint, setHistoryData, refreshAllCharts, getHistoryData } from './charts.js';

let lastTimestamp = 0;
let currentVideoBlobUrl = null;
let currentPhotoBlobUrl = null;

export function isValidSensorData(data) {
    if (data.temperature === -999 || data.humidity === -999 || data.uvi === -999 || data.lux === -999) {
        console.warn("Sensor data contains -999 (error), still displaying but may be invalid.");
    }
    if (data.temperature < -40 || data.temperature > 80) return false;
    if (data.humidity < 0 || data.humidity > 100) return false;
    if (data.uvi < 0 || data.uvi > 20) return false;
    if (data.lux < 0 || data.lux > 20000) return false;
    if (Math.abs(data.ax) > 8 || Math.abs(data.ay) > 8 || Math.abs(data.az) > 8) return false;
    return true;
}

export function updateSensorPanel(data) {
    if (!data) return;
    const mag = Math.sqrt(data.ax * data.ax + data.ay * data.ay + data.az * data.az);
    document.getElementById('sensor-panel').innerHTML = `
        <div class="sensor-item"><div class="sensor-label">Temperature</div><div class="sensor-value">${data.temperature.toFixed(1)} °C</div></div>
        <div class="sensor-item"><div class="sensor-label">Humidity</div><div class="sensor-value">${Math.round(data.humidity)} %</div></div>
        <div class="sensor-item"><div class="sensor-label">UV Index</div><div class="sensor-value">${data.uvi.toFixed(2)}</div></div>
        <div class="sensor-item"><div class="sensor-label">Illuminance</div><div class="sensor-value">${Math.round(data.lux)} lx</div></div>
        <div class="sensor-item"><div class="sensor-label">Accel X</div><div class="sensor-value">${data.ax.toFixed(1)} g</div></div>
        <div class="sensor-item"><div class="sensor-label">Accel Y</div><div class="sensor-value">${data.ay.toFixed(1)} g</div></div>
        <div class="sensor-item"><div class="sensor-label">Accel Z</div><div class="sensor-value">${data.az.toFixed(1)} g</div></div>
        <div class="sensor-item"><div class="sensor-label">Resultant Accel</div><div class="sensor-value">${mag.toFixed(2)} g</div></div>
    `;
    updateArrow(data.ax, data.ay, data.az);
}

export function processSensorData(data) {
    if (data.timestamp) data.timestamp = toSeconds(data.timestamp);
    else data.timestamp = Date.now() / 1000;
    if (isValidSensorData(data)) {
        lastTimestamp = data.timestamp;
        updateSensorPanel(data);
        addDataPoint(data);
    } else {
        console.warn("Invalid sensor data discarded:", data);
    }
}

export function processHistory(historyArray) {
    const valid = [];
    for (let item of historyArray) {
        if (item.timestamp) item.timestamp = toSeconds(item.timestamp);
        else item.timestamp = Date.now() / 1000;
        if (isValidSensorData(item)) {
            valid.push(item);
        }
    }
    while (valid.length > MAX_HISTORY) valid.shift();
    setHistoryData(valid);
    if (valid.length) {
        const latest = valid[valid.length - 1];
        updateSensorPanel(latest);
        lastTimestamp = latest.timestamp;
        refreshAllCharts();
    }
}

export function processVideoFrame(binaryData) {
    if (binaryData.length < 2) return;
    const jpegData = binaryData.slice(1);
    const blob = new Blob([jpegData], { type: 'image/jpeg' });
    const url = URL.createObjectURL(blob);
    const videoImg = document.getElementById('videoStream');
    videoImg.onload = () => { URL.revokeObjectURL(url); };
    videoImg.src = url;
    videoImg.style.display = 'block';
    document.getElementById('videoPlaceholder').style.display = 'none';
    if (currentVideoBlobUrl) URL.revokeObjectURL(currentVideoBlobUrl);
    currentVideoBlobUrl = url;
}

export function processPhoto(binaryData) {
    if (binaryData.length < 2) return;
    const jpegData = binaryData.slice(1);
    const blob = new Blob([jpegData], { type: 'image/jpeg' });
    const url = URL.createObjectURL(blob);
    const photoImg = document.getElementById('latestPhoto');
    photoImg.onload = () => { URL.revokeObjectURL(url); };
    photoImg.src = url;
    photoImg.style.display = 'block';
    document.getElementById('photoPlaceholder').style.display = 'none';
    if (currentPhotoBlobUrl) URL.revokeObjectURL(currentPhotoBlobUrl);
    currentPhotoBlobUrl = url;
}
