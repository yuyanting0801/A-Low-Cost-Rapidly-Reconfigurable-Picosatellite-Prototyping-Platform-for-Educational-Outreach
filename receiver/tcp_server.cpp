#include "tcp_server.h"
#include "globals.h"
#include "config.h"
#include "sd_card.h"
#include "websocket.h"
#include <SD.h>

/*
 * TCP upload server implementation.
 */

/** Read one line of text from the client (up to \n) */
String readLine(WiFiClient& client, unsigned long timeout = 5000) {
  String line = "";
  unsigned long start = millis();
  while (millis() - start < timeout) {
    if (client.available()) {
      char c = client.read();
      if (c == '\r') continue;
      if (c == '\n') break;
      line += c;
    }
    vTaskDelay(1);
  }
  return line;
}

/** Extract one header's value from the raw header block */
String getHeaderValue(const String& headers, const String& headerName) {
  String search = headerName + ": ";
  int pos = headers.indexOf(search);
  if (pos < 0) return "";
  pos += search.length();
  int end = headers.indexOf("\r\n", pos);
  if (end < 0) return "";
  return headers.substring(pos, end);
}

/** Parse a POST /upload request: X-File-Name, X-Sensor-Data and Content-Length */
bool parseHTTPRequest(WiFiClient& client, String& filename, String& sensorData, size_t& contentLength) {
  String requestLine = readLine(client);
  if (requestLine.indexOf("POST /upload") < 0) return false;

  String headers = "";
  String line;
  while ((line = readLine(client)).length() > 0) {
    headers += line + "\r\n";
  }

  filename = getHeaderValue(headers, "X-File-Name");
  if (filename.length() == 0) filename = getHeaderValue(headers, "x-file-name");

  sensorData = getHeaderValue(headers, "X-Sensor-Data");
  if (sensorData.length() == 0) sensorData = getHeaderValue(headers, "x-sensor-data");

  String cl = getHeaderValue(headers, "Content-Length");
  contentLength = cl.toInt();

  return (filename.length() > 0 && contentLength > 0);
}

/** TCP server task: serve POST /upload on port 8080 */
void tcpServerTask(void* pvParameters) {
  WiFiServer tcpServer(TCP_PORT);
  tcpServer.begin();
  tcpServer.setNoDelay(true);
  Serial.println("TCP server started");

  WiFiClient client;

  while (1) {
    if (!client.connected()) {
      client = tcpServer.available();
      if (client) {
        Serial.println("New TCP client connected");
        client.setTimeout(30000);
      }
    } else {
      String filename, sensorData;
      size_t contentLength = 0;
      if (!parseHTTPRequest(client, filename, sensorData, contentLength)) {
        client.print("HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
        client.stop();
        continue;
      }

      if (!sdReady) {
        client.print("HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\nSD Card error");
        client.stop();
        continue;
      }

      // Store the photo on the SD card
      String imagePath = "/images/" + filename;
      if (SD.exists(imagePath)) SD.remove(imagePath);
      File imgFile = SD.open(imagePath, FILE_WRITE);
      if (!imgFile) {
        client.print("HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\nCannot create file");
        client.stop();
        continue;
      }

      size_t received = 0;
      uint8_t buffer[4096];
      while (received < contentLength && client.connected()) {
        int avail = client.available();
        if (avail > 0) {
          size_t toRead = (avail < (int)sizeof(buffer)) ? avail : sizeof(buffer);
          size_t rd = client.read(buffer, toRead);
          if (rd > 0) {
            imgFile.write(buffer, rd);
            received += rd;
          }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
      }
      imgFile.close();

      if (received != contentLength) {
        SD.remove(imagePath);
        client.print("HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\nIncomplete transfer");
        client.stop();
        continue;
      }

      // Store the matching sensor JSON next to the photo
      String sensorFilename = filename.substring(0, filename.lastIndexOf('.')) + ".json";
      String sensorPath = "/sensor_data/" + sensorFilename;
      File jsonFile = SD.open(sensorPath, FILE_WRITE);
      if (jsonFile) {
        jsonFile.print(sensorData);
        jsonFile.close();
      }

      latestPhotoPath = imagePath;
      Serial.printf("Saved: %s (%d bytes)\n", imagePath.c_str(), received);
      client.print("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nOK");
      client.stop();

      // Push the new photo to the backend over WebSocket
      sendPhotoViaWS(imagePath);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
