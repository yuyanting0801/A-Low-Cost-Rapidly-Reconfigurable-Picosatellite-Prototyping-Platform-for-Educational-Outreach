#include "sensors.h"
#include "websocket.h"
#include "../lib/IWA.h"
#include "../lib/ILB.h"
#include "../lib/IIA.h"

/*
 * Sensor module: init, thread-safe readout and periodic reporting task.
 */

IWA iwa;
ILB ilb;
IIA iia;

/** Initialize all I2C sensors, retrying up to 3 times each */
bool initSensors() {
  bool ok = true;
  Serial.println("Initializing sensors...");
  for (int retry = 0; retry < 3; retry++) {
    if (iwa.begin()) break;
    if (retry == 2) { Serial.println("IWA failed"); ok = false; }
    delay(500);
  }
  for (int retry = 0; retry < 3; retry++) {
    if (ilb.begin()) break;
    if (retry == 2) { Serial.println("ILB failed"); ok = false; }
    delay(500);
  }
  for (int retry = 0; retry < 3; retry++) {
    if (iia.begin()) break;
    if (retry == 2) { Serial.println("IIA failed"); ok = false; }
    delay(500);
  }
  if (ok) Serial.println("All sensors OK");
  else    Serial.println("Some sensors missing");
  return ok;
}

/** Thread-safe sensor readout (takes the I2C mutex) */
bool safeReadSensorData(SensorData& data) {
  if (!sensorsInitialized) return false;
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
    Serial.println("I2C mutex timeout");
    return false;
  }
  bool ok = true;
  if (!iwa.getData(data.temperature, data.humidity)) ok = false;
  if (!ilb.getData(data.uvi, data.lux)) ok = false;
  if (!iia.getData(data.ax, data.ay, data.az)) ok = false;
  data.timestamp = millis() / 1000;
  xSemaphoreGive(i2cMutex);
  return ok;
}

/** Sensor task: read every 3 s, cache the latest values and push them to the backend */
void sensorTask(void* pvParameters) {
  Serial.println("Sensor task started (3s interval)");
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t sensorInterval = pdMS_TO_TICKS(3000);

  while (1) {
    SensorData data;
    if (safeReadSensorData(data)) {
      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        latestSensorData = data;
        xSemaphoreGive(i2cMutex);
      }
      Serial.printf("Sensor: %.1fC, %.1f%%, %.2f UVI, %.0f Lux\n",
                    data.temperature, data.humidity, data.uvi, data.lux);
      sendSensorDataViaWS(data);
      Serial.printf("Free heap: %d, PSRAM: %d, Queue items: %d\n",
                    ESP.getFreeHeap(), ESP.getFreePsram(), uxQueueMessagesWaiting(wsSendQueue));

      static unsigned long lastHeapCheck = 0;
      if (millis() - lastHeapCheck > 30000) {
        lastHeapCheck = millis();
        if (!heap_caps_check_integrity(MALLOC_CAP_INTERNAL, true)) {
          Serial.println("Heap corruption detected!");
        }
      }
    } else {
      Serial.println("Failed to read sensor data");
    }
    vTaskDelayUntil(&lastWakeTime, sensorInterval);
  }
}
