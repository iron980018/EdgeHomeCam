#include "upload_service.h"

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
  WiFiClient plainClient;
  HTTPClient http;

  const bool isHttps = String(UPLOAD_URL).startsWith("https://");
#if HTTP_ALLOW_INSECURE_TLS
  secureClient.setInsecure();
#endif

  bool started = isHttps ? http.begin(secureClient, UPLOAD_URL)
                         : http.begin(plainClient, UPLOAD_URL);
  if (!started) {
    errorCode = "UPLOAD_BEGIN_FAILED";
    return false;
  }

  http.setTimeout(UPLOAD_TIMEOUT_MS);

  if (strlen(UPLOAD_API_KEY) > 0) {
    http.addHeader("Authorization", String("Bearer ") + UPLOAD_API_KEY);
  }

  String boundary = "----EdgeCamBoundary7MA4YWxkTrZu0gW";
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

  String head;
  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"device_id\"\r\n\r\n";
  head += DEVICE_ID;
  head += "\r\n--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"captured_at\"\r\n\r\n";
  head += capturedAt;
  head += "\r\n--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"battery_voltage\"\r\n\r\n";
  head += String(batteryVoltage, 2);
  head += "\r\n--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"wifi_rssi\"\r\n\r\n";
  head += String(wifiRssi);
  head += "\r\n--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"firmware_version\"\r\n\r\n";
  head += FIRMWARE_VERSION;
  head += "\r\n--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"image\"; filename=\"capture.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";

  String tail = "\r\n--" + boundary + "--\r\n";

  size_t totalLength = head.length() + frame->len + tail.length();
  uint8_t *body = static_cast<uint8_t *>(malloc(totalLength));
  if (!body) {
    http.end();
    errorCode = "UPLOAD_OUT_OF_MEMORY";
    return false;
  }

  memcpy(body, head.c_str(), head.length());
  memcpy(body + head.length(), frame->buf, frame->len);
  memcpy(body + head.length() + frame->len, tail.c_str(), tail.length());

  int code = http.POST(body, totalLength);
  free(body);

  String response = http.getString();
  http.end();

  Serial.printf("Upload response code: %d\n", code);
  Serial.println(response);

  if (code < 200 || code >= 300) {
    errorCode = "UPLOAD_FAILED";
    return false;
  }

  imageUrl = extractImageUrl(response);
  return true;
}

String UploadService::extractImageUrl(const String &body) const {
  const String key = "\"image_url\"";
  int keyIndex = body.indexOf(key);
  if (keyIndex < 0) {
    return "";
  }

  int colon = body.indexOf(':', keyIndex + key.length());
  int firstQuote = body.indexOf('"', colon + 1);
  int secondQuote = body.indexOf('"', firstQuote + 1);
  if (colon < 0 || firstQuote < 0 || secondQuote < 0) {
    return "";
  }

  return body.substring(firstQuote + 1, secondQuote);
}
