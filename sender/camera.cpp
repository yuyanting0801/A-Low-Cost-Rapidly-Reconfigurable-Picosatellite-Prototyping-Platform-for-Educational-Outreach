#include "camera.h"
#include "globals.h"
#include "sensors.h"
#include "websocket.h"
#include <esp_camera.h>
#include "../lib/board_config.h"

/*
 * Camera module: init, ~20 fps streaming capture task and single-shot photo task.
 */

/** Initialize the OV2640: QVGA JPEG frames, 2 frame buffers in PSRAM */
bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size   = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode    = CAMERA_GRAB_LATEST;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 18;
  config.fb_count     = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }
  sensor_t* s = esp_camera_sensor_get();
  if (s->id.PID == OV2640_PID) {
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
  }
  Serial.println("Camera ready");
  return true;
}

/** Streaming task: grab JPEG frames at ~20 fps, queue them for MJPEG and send over WebSocket */
void captureFrameTask(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t frameInterval = pdMS_TO_TICKS(66);  // ~20 fps

  while (1) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    int idx;
    if (xSemaphoreTake(frameMutex, portMAX_DELAY) == pdTRUE) {
      idx = currentWriteBuffer;
      currentWriteBuffer = (currentWriteBuffer + 1) % NUM_FRAME_BUFFERS;
      xSemaphoreGive(frameMutex);
    } else {
      esp_camera_fb_return(fb);
      continue;
    }

    FrameBuffer* fbBuf = &frameBuffers[idx];
    // Drop frames that would overflow the preallocated 25 KB buffer
    if (fb->len > 25 * 1024) {
      esp_camera_fb_return(fb);
      continue;
    }
    memcpy(fbBuf->buffer, fb->buf, fb->len);
    fbBuf->length = fb->len;
    esp_camera_fb_return(fb);

    // Queue the frame for the HTTP MJPEG stream
    xQueueSend(frameQueue, &fbBuf, 0);
    sendFrameViaWS(fbBuf->buffer, fbBuf->length);

    vTaskDelayUntil(&lastWakeTime, frameInterval);
  }
}

/** Photo task: on notification, capture one JPEG with sensor data and queue it for upload */
void captureTask(void* pvParameters) {
  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    Serial.println("Capturing photo...");

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Capture failed");
      continue;
    }

    TransmitPacket* pkt = (TransmitPacket*)ps_malloc(sizeof(TransmitPacket));
    if (!pkt) {
      esp_camera_fb_return(fb);
      continue;
    }
    pkt->imageData = (uint8_t*)ps_malloc(fb->len);
    if (!pkt->imageData) {
      free(pkt);
      esp_camera_fb_return(fb);
      continue;
    }
    memcpy(pkt->imageData, fb->buf, fb->len);
    pkt->imageSize = fb->len;

    if (!safeReadSensorData(pkt->sensorData)) {
      pkt->sensorData.temperature = -999;
      pkt->sensorData.humidity    = -999;
      pkt->sensorData.uvi         = -999;
      pkt->sensorData.lux         = -999;
      pkt->sensorData.ax = pkt->sensorData.ay = pkt->sensorData.az = 0;
      pkt->sensorData.timestamp = millis() / 1000;
    }
    esp_camera_fb_return(fb);

    if (xQueueSend(transmitQueue, &pkt, pdMS_TO_TICKS(1000)) != pdTRUE) {
      free(pkt->imageData);
      free(pkt);
    }
  }
}
