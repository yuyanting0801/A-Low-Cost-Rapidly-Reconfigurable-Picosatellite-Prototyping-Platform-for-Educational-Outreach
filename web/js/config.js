/**
 * Frontend configuration: WebSocket URL, reconnect backoff limits, chart ranges, device IPs.
 */
export const WS_URL = `ws://${window.location.hostname}:5001`;
export const MAX_RECONNECT_DELAY = 30000;
export const BASE_RECONNECT_DELAY = 1000;
export const MAX_HISTORY = 30;
export const TEMP_MIN = -10, TEMP_MAX = 40;
export const HUMID_MIN = 0, HUMID_MAX = 100;
export const LUX_MIN = 0, LUX_MAX = 5000;
export const UV_MIN = 0, UV_MAX = 12;
// Device IPs: the static IPs you assigned to the two boards
export const SENDER_IP = "SENDER_IP";
export const RECEIVER_IP = "RECEIVER_IP";

/** Normalise a timestamp to seconds (the server sends milliseconds). */
export function toSeconds(ts) {
    if (ts === undefined || ts === null) return Date.now() / 1000;
    if (ts > 1e12) return ts / 1000;
    return ts;
}
