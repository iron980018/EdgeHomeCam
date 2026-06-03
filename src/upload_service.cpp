#include "upload_service.h"

#include <ctype.h>
#include <WiFiClientSecure.h>
#include "config.h"

bool UploadService::upload(camera_fb_t *frame, const String &capturedAt,
                           float batteryVoltage, int wifiRssi, String &imageUrl,
                           String &errorCode) {
  if (!frame || frame->len == 0) {
    errorCode = "CAPTURE_FAILED";
    return false;
  }

  if (frame->len > TELEGRAM_MAX_PHOTO_BYTES) {
    errorCode = "TELEGRAM_IMAGE_TOO_LARGE";
    return false;
  }

  WiFiClientSecure secureClient;

#if TELEGRAM_ALLOW_INSECURE_TLS
  secureClient.setInsecure();
#endif

  secureClient.setTimeout(TELEGRAM_TIMEOUT_MS / 1000UL);
  if (!secureClient.connect("api.telegram.org", 443)) {
    errorCode = "TELEGRAM_CONNECT_FAILED";
    return false;
  }

  String boundary = "----EdgeCamBoundary7MA4YWxkTrZu0gW";
  String head;
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
  head += TELEGRAM_CHAT_ID;
  head += "\r\n--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n";
  head += buildCaption(capturedAt, batteryVoltage, wifiRssi);
  head += "\r\n--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"photo\"; filename=\"edgecam.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";

  String tail = "\r\n--" + boundary + "--\r\n";

  size_t totalLength = head.length() + frame->len + tail.length();
  secureClient.print(String("POST /bot") + TELEGRAM_BOT_TOKEN + "/sendPhoto HTTP/1.1\r\n");
  secureClient.print("Host: api.telegram.org\r\n");
  secureClient.print(String("User-Agent: EdgeCam/") + FIRMWARE_VERSION + "\r\n");
  secureClient.print("Connection: close\r\n");
  secureClient.print("Content-Type: multipart/form-data; boundary=" + boundary + "\r\n");
  secureClient.print("Content-Length: " + String(totalLength) + "\r\n\r\n");

  secureClient.print(head);
  size_t photoBytesWritten = secureClient.write(frame->buf, frame->len);
  secureClient.print(tail);

  if (photoBytesWritten != frame->len) {
    secureClient.stop();
    errorCode = "TELEGRAM_SEND_FAILED";
    return false;
  }

  String statusLine = secureClient.readStringUntil('\n');
  int code = parseStatusCode(statusLine);
  String response = readResponseBody(secureClient);
  secureClient.stop();

  Serial.printf("Telegram response code: %d\n", code);
  Serial.println(response);

  if (code < 200 || code >= 300) {
    errorCode = "TELEGRAM_SEND_FAILED";
    return false;
  }

  if (response.indexOf("\"ok\":true") < 0) {
    errorCode = "TELEGRAM_API_FAILED";
    return false;
  }

  String messageId = extractMessageId(response);
  imageUrl = messageId.length() > 0 ? String("telegram:message/") + messageId : "telegram:sent";
  return true;
}

String UploadService::buildCaption(const String &capturedAt, float batteryVoltage,
                                   int wifiRssi) const {
  String caption;
  caption += "EdgeCam gas meter capture\n";
  caption += "Device: ";
  caption += DEVICE_ID;
  caption += "\nTime: ";
  caption += capturedAt;
  caption += "\nBattery: ";
  caption += String(batteryVoltage, 2);
  caption += " V\nRSSI: ";
  caption += String(wifiRssi);
  caption += " dBm\nFirmware: ";
  caption += FIRMWARE_VERSION;
  return caption;
}

int UploadService::parseStatusCode(const String &statusLine) const {
  int firstSpace = statusLine.indexOf(' ');
  if (firstSpace < 0 || firstSpace + 4 > statusLine.length()) {
    return -1;
  }

  return statusLine.substring(firstSpace + 1, firstSpace + 4).toInt();
}

String UploadService::readResponseBody(WiFiClientSecure &client) const {
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r" || line.length() == 0) {
      break;
    }
  }

  String body;
  unsigned long startedAt = millis();
  while (millis() - startedAt < TELEGRAM_TIMEOUT_MS) {
    while (client.available()) {
      body += static_cast<char>(client.read());
      startedAt = millis();
    }

    if (!client.connected()) {
      break;
    }

    delay(10);
  }

  return body;
}

String UploadService::extractMessageId(const String &body) const {
  const String key = "\"message_id\"";
  int keyIndex = body.indexOf(key);
  if (keyIndex < 0) {
    return "";
  }

  int colon = body.indexOf(':', keyIndex + key.length());
  if (colon < 0) {
    return "";
  }

  int start = colon + 1;
  while (start < body.length() && isspace(static_cast<unsigned char>(body[start]))) {
    start++;
  }

  int end = start;
  while (end < body.length() && isdigit(static_cast<unsigned char>(body[end]))) {
    end++;
  }

  return body.substring(start, end);
}
