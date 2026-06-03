#pragma once

#include <Arduino.h>
#include "esp_camera.h"

class UploadService {
 public:
  bool upload(camera_fb_t *frame, const String &capturedAt, float batteryVoltage,
              int wifiRssi, String &imageUrl, String &errorCode);

 private:
  String telegramApiUrl() const;
  String buildCaption(const String &capturedAt, float batteryVoltage, int wifiRssi) const;
  String extractMessageId(const String &body) const;
};
