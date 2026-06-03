#include "power_manager.h"

#include "config.h"

void PowerManager::begin() {
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

#if DEEP_SLEEP_EXT_WAKEUP_ENABLED
  pinMode(static_cast<uint8_t>(WAKEUP_PIN), INPUT_PULLUP);
#endif

#if EXTERNAL_POWER_CONTROL_ENABLED
  pinMode(POWER_DONE_PIN, OUTPUT);
  digitalWrite(POWER_DONE_PIN, LOW);
#endif

#if BATTERY_ADC_ENABLED
  analogReadResolution(12);
  analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);
#endif
}

float PowerManager::batteryVoltage() const {
#if BATTERY_ADC_ENABLED
  uint32_t raw = analogRead(BATTERY_ADC_PIN);
  float adcVoltage = (raw / 4095.0f) * 3.3f;
  return adcVoltage * BATTERY_DIVIDER_RATIO;
#else
  return 0.0f;
#endif
}

bool PowerManager::isLowBattery() const {
#if BATTERY_ADC_ENABLED
  return batteryVoltage() > 0.1f && batteryVoltage() < LOW_BATTERY_VOLTAGE;
#else
  return false;
#endif
}

esp_sleep_wakeup_cause_t PowerManager::wakeupCause() const {
  return esp_sleep_get_wakeup_cause();
}

void PowerManager::finishAndPowerOff() {
#if EXTERNAL_POWER_CONTROL_ENABLED
  Serial.println("Signaling external power controller DONE");
  digitalWrite(FLASH_LED_PIN, LOW);
  digitalWrite(POWER_DONE_PIN, HIGH);
  delay(POWER_DONE_SIGNAL_MS);
#endif
}

void PowerManager::sleepNow(uint64_t seconds) {
  Serial.printf("Entering deep sleep for %llu seconds\n", seconds);
  Serial.flush();

  digitalWrite(FLASH_LED_PIN, LOW);
  finishAndPowerOff();
  esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
#if DEEP_SLEEP_EXT_WAKEUP_ENABLED
  esp_sleep_enable_ext0_wakeup(WAKEUP_PIN, 0);
#endif
  esp_deep_sleep_start();
}
