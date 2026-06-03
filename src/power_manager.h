#pragma once

#include <Arduino.h>
#include "esp_sleep.h"

class PowerManager {
 public:
  void begin();
  float batteryVoltage() const;
  bool isLowBattery() const;
  esp_sleep_wakeup_cause_t wakeupCause() const;
  void finishAndPowerOff();
  void sleepNow(uint64_t seconds);
};
