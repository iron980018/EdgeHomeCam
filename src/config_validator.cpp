#include "config_validator.h"

#include <string.h>
#include "config.h"

bool ConfigValidator::hasRequiredBootConfig(String &errorCode) {
  if (isPlaceholder(WIFI_SSID) || isPlaceholder(WIFI_PASSWORD)) {
    errorCode = "CONFIG_WIFI_INVALID";
    return false;
  }

  if (isPlaceholder(BLYNK_AUTH_TOKEN) || isPlaceholder(BLYNK_TEMPLATE_ID)) {
    errorCode = "CONFIG_BLYNK_INVALID";
    return false;
  }

  return true;
}

bool ConfigValidator::hasRequiredCaptureConfig(String &errorCode) {
  if (isPlaceholder(TELEGRAM_BOT_TOKEN) || isPlaceholder(TELEGRAM_CHAT_ID)) {
    errorCode = "CONFIG_TELEGRAM_INVALID";
    return false;
  }

  return true;
}

bool ConfigValidator::isPlaceholder(const char *value) {
  if (!value || strlen(value) == 0) {
    return true;
  }

  String text(value);
  return text.startsWith("YOUR_") || text.startsWith("TMPLxxxx");
}
