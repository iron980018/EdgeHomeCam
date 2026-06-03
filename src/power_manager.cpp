#include "power_manager.h"

#include "config.h"

void PowerManager::begin() {
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);
  pinMode(static_cast<uint8_t>(WAKEUP_PIN), INPUT_PULLUP);

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

bool PowerManager::wokeFromExternalPin() const {
  return wakeupCause() == ESP_SLEEP_WAKEUP_EXT0 || wakeupCause() == ESP_SLEEP_WAKEUP_EXT1;
}

void PowerManager::sleepNow(uint64_t seconds) {
  Serial.printf("Entering deep sleep for %llu seconds\n", seconds);
  Serial.flush();

  digitalWrite(FLASH_LED_PIN, LOW);
  esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
  esp_sleep_enable_ext0_wakeup(WAKEUP_PIN, 0);
  esp_deep_sleep_start();
}
