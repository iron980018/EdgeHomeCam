#pragma once

#include <Arduino.h>

class ConfigValidator {
 public:
  static bool hasRequiredBootConfig(String &errorCode);
  static bool hasRequiredCaptureConfig(String &errorCode);

 private:
  static bool isPlaceholder(const char *value);
};
