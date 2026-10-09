/**
 * Chart module (Chart.js): weather, illuminance and UV charts, all fed from
 * one bounded history buffer.
 */
import { TEMP_MIN, TEMP_MAX, HUMID_MIN, HUMID_MAX, LUX_MIN, LUX_MAX, UV_MIN, UV_MAX, MAX_HISTORY, toSeconds } from './config.js';

let weatherChart, luxChart, uvChart;
let historyData = [];

export function initCharts() {
    weatherChart = new Chart(document.getElementById('weatherChart'), {
        type: 'line',
        data: {
            datasets: [
                { label: 'Temperature (°C)', borderColor: '#e3b5a4', data: [], yAxisID: 'y-temp', tension: 0.3 },
                { label: 'Humidity (%)', type: 'bar', borderColor: '#b7c2a9', backgroundColor: 'rgba(183,194,169,0.6)', data: [], yAxisID: 'y-humid' }
            ]
        },
        options: {
            responsive: true, maintainAspectRatio: false,
            scales: {
                x: { title: { display: true, text: 'Time (seconds ago)' } },
                'y-temp': { position: 'left', min: TEMP_MIN, max: TEMP_MAX, title: { display: true, text: 'Temperature (°C)' } },
                'y-humid': { position: 'right', grid: { drawOnChartArea: false }, min: HUMID_MIN, max: HUMID_MAX, title: { display: true, text: 'Humidity (%)' } }
            }
        }
    });
    luxChart = new Chart(document.getElementById('luxChart'), {
        type: 'line',
        data: { datasets: [{ label: 'Illuminance (lx)', borderColor: '#d9c8b2', data: [], tension: 0.3 }] },
        options: { responsive: true, maintainAspectRatio: false, scales: { x: { title: { display: true, text: 'Time (seconds ago)' } }, y: { min: LUX_MIN, max: LUX_MAX, title: { display: true, text: 'Illuminance (lx)' } } } }
    });
    uvChart = new Chart(document.getElementById('uvChart'), {
        type: 'line',
        data: { datasets: [{ label: 'UV Index', borderColor: '#c7b9a5', data: [], tension: 0.3 }] },
        options: { responsive: true, maintainAspectRatio: false, scales: { x: { title: { display: true, text: 'Time (seconds ago)' } }, y: { min: UV_MIN, max: UV_MAX, title: { display: true, text: 'UV Index' } } } }
    });
}

export function addDataPoint(point) {
    if (!point) return;
    point.timestamp = toSeconds(point.timestamp);
    historyData.push(point);
    if (historyData.length > MAX_HISTORY) historyData.shift();
    refreshAllCharts();
}

export function setHistoryData(data) {
    historyData = data;
}

export function getHistoryData() {
    return historyData;
}

export function refreshAllCharts() {
    if (historyData.length === 0) return;
    const nowSec = Date.now() / 1000;
    const labels = historyData.map(p => {
        let diff = nowSec - p.timestamp;
        diff = Math.max(0, diff).toFixed(1);
        return `${diff}s ago`;
    });
    weatherChart.data.labels = labels;
    weatherChart.data.datasets[0].data = historyData.map(p => p.temperature);
    weatherChart.data.datasets[1].data = historyData.map(p => p.humidity);
    weatherChart.update();
    luxChart.data.labels = labels;
    luxChart.data.datasets[0].data = historyData.map(p => p.lux);
    luxChart.update();
    uvChart.data.labels = labels;
    uvChart.data.datasets[0].data = historyData.map(p => p.uvi);
    uvChart.update();
}
