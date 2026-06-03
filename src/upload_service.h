#pragma once

#include <Arduino.h>
#include "esp_camera.h"

class WiFiClientSecure;

class UploadService {
 public:
  bool upload(camera_fb_t *frame, const String &capturedAt, float batteryVoltage,
              int wifiRssi, String &imageUrl, String &errorCode);

 private:
  String buildCaption(const String &capturedAt, float batteryVoltage, int wifiRssi) const;
  int parseStatusCode(const String &statusLine) const;
  String readResponseBody(WiFiClientSecure &client) const;
  String extractMessageId(const String &body) const;
};
