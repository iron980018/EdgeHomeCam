#include "upload_service.h"

#include <ctype.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "config.h"

bool UploadService::upload(camera_fb_t *frame, const String &capturedAt,
                           float batteryVoltage, int wifiRssi, String &imageUrl,
                           String &errorCode) {
  if (!frame || frame->len == 0) {
    errorCode = "CAPTURE_FAILED";
    return false;
  }

  WiFiClientSecure secureClient;
  HTTPClient http;

#if TELEGRAM_ALLOW_INSECURE_TLS
  secureClient.setInsecure();
#endif

  String url = telegramApiUrl();
  if (!http.begin(secureClient, url)) {
    errorCode = "TELEGRAM_BEGIN_FAILED";
    return false;
  }

  http.setTimeout(TELEGRAM_TIMEOUT_MS);

  String boundary = "----EdgeCamBoundary7MA4YWxkTrZu0gW";
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

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
  uint8_t *body = static_cast<uint8_t *>(malloc(totalLength));
  if (!body) {
    http.end();
    errorCode = "TELEGRAM_OUT_OF_MEMORY";
    return false;
  }

  memcpy(body, head.c_str(), head.length());
  memcpy(body + head.length(), frame->buf, frame->len);
  memcpy(body + head.length() + frame->len, tail.c_str(), tail.length());

  int code = http.POST(body, totalLength);
  free(body);

  String response = http.getString();
  http.end();

  Serial.printf("Telegram response code: %d\n", code);
  Serial.println(response);

  if (code < 200 || code >= 300) {
    errorCode = "TELEGRAM_SEND_FAILED";
    return false;
  }

  String messageId = extractMessageId(response);
  imageUrl = messageId.length() > 0 ? String("telegram:message/") + messageId : "telegram:sent";
  return true;
}

String UploadService::telegramApiUrl() const {
  return String("https://api.telegram.org/bot") + TELEGRAM_BOT_TOKEN + "/sendPhoto";
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
