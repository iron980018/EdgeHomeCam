#include "rtc_manager.h"

#include <Wire.h>
#include "config.h"

#if RTC_ENABLED
#include <RTClib.h>

#if RTC_TYPE == RTC_TYPE_DS3231
static RTC_DS3231 rtcDevice;
#elif RTC_TYPE == RTC_TYPE_PCF8523
static RTC_PCF8523 rtcDevice;
#else
#error "Unsupported RTC_TYPE. Use RTC_TYPE_DS3231 or RTC_TYPE_PCF8523."
#endif
#endif

bool RtcManager::begin() {
#if RTC_ENABLED
  Wire.begin(RTC_SDA_PIN, RTC_SCL_PIN);
  ready_ = rtcDevice.begin();
  if (!ready_) {
    Serial.println("RTC init failed");
    validTime_ = false;
    return false;
  }

  validTime_ = true;
#if RTC_TYPE == RTC_TYPE_DS3231
  if (rtcDevice.lostPower()) {
    Serial.println("RTC lost power; time may be invalid");
    validTime_ = false;
  }
#elif RTC_TYPE == RTC_TYPE_PCF8523
  if (rtcDevice.lostPower()) {
    Serial.println("RTC lost power; time may be invalid");
    validTime_ = false;
  }
#endif

  return true;
#else
  ready_ = false;
  validTime_ = false;
  return false;
#endif
}

bool RtcManager::isReady() const {
  return ready_;
}

bool RtcManager::hasValidTime() const {
  return validTime_;
}

bool RtcManager::isWithinActiveWindow() const {
#if RTC_ENABLED && ACTIVE_WINDOW_ENABLED
  if (!ready_ || !validTime_) {
    return true;
  }

  DateTime now = rtcDevice.now();
  uint8_t hour = now.hour();

  if (ACTIVE_START_HOUR == ACTIVE_END_HOUR) {
    return true;
  }

  if (ACTIVE_START_HOUR < ACTIVE_END_HOUR) {
    return hour >= ACTIVE_START_HOUR && hour < ACTIVE_END_HOUR;
  }

  return hour >= ACTIVE_START_HOUR || hour < ACTIVE_END_HOUR;
#else
  return true;
#endif
}

String RtcManager::nowString() const {
#if RTC_ENABLED
  if (!ready_ || !validTime_) {
    return "";
  }

  DateTime now = rtcDevice.now();
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d",
           now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  return String(buffer);
#else
  return "";
#endif
}
