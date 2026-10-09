#include "http_server.h"
#include "globals.h"
#include "config.h"
#include "sd_card.h"
#include <SD.h>

/*
 * HTTP file server implementation.
 */

void httpServerTask(void* pvParameters) {
  WiFiServer httpServer(HTTP_PORT);
  httpServer.begin();
  Serial.println("HTTP server started");

  WiFiClient client;

  while (1) {
    if (!client.connected()) {
      client = httpServer.available();
      if (client) {
        Serial.println("HTTP client connected");
        client.setTimeout(5000);
      }
    } else {
      String request = client.readStringUntil('\r');
      client.readStringUntil('\n');

      String path = "";
      int firstSpace = request.indexOf(' ');
      int secondSpace = request.indexOf(' ', firstSpace + 1);
      if (firstSpace != -1 && secondSpace != -1) {
        path = request.substring(firstSpace + 1, secondSpace);
      }

      if (path == "/latest_photo") {
        if (latestPhotoPath.length() > 0 && SD.exists(latestPhotoPath)) {
          File photo = SD.open(latestPhotoPath, FILE_READ);
          if (photo) {
            client.print("HTTP/1.1 200 OK\r\n");
            client.print("Content-Type: image/jpeg\r\n");
            client.print("Content-Length: ");
            client.print(photo.size());
            client.print("\r\n\r\n");
            uint8_t buffer[1024];
            size_t bytesRead;
            while ((bytesRead = photo.read(buffer, sizeof(buffer))) > 0) {
              client.write(buffer, bytesRead);
            }
            photo.close();
          } else {
            client.print("HTTP/1.1 404 Not Found\r\n\r\n");
          }
        } else {
          client.print("HTTP/1.1 404 Not Found\r\n\r\n");
        }
      }
      else if (path == "/photos") {
        String json = getPhotoListJson();
        client.print("HTTP/1.1 200 OK\r\n");
        client.print("Content-Type: application/json\r\n");
        client.print("Content-Length: ");
        client.print(json.length());
        client.print("\r\n\r\n");
        client.print(json);
      }
      else if (path == "/sensor_data") {
        String json = getSensorDataListJson();
        client.print("HTTP/1.1 200 OK\r\n");
        client.print("Content-Type: application/json\r\n");
        client.print("Content-Length: ");
        client.print(json.length());
        client.print("\r\n\r\n");
        client.print(json);
      }
      else {
        client.print("HTTP/1.1 404 Not Found\r\n\r\n");
      }

      client.stop();
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
