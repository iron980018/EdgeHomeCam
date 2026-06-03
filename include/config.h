#pragma once

// Blynk definitions must be available before including BlynkSimpleEsp32.h.
#define BLYNK_TEMPLATE_ID "TMPLxxxx"
#define BLYNK_TEMPLATE_NAME "EdgeCam"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

// Device identity
#define DEVICE_ID "gas-meter-cam-001"
#define FIRMWARE_VERSION "0.1.0"

// Wi-Fi
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define WIFI_RETRY_COUNT 5
#define WIFI_CONNECT_TIMEOUT_MS 30000UL

// Blynk
#define BLYNK_CONNECT_TIMEOUT_MS 15000UL
#define BLYNK_RUN_INTERVAL_MS 10UL
#define BLYNK_SYNC_TIMEOUT_MS 3000UL
#define BLYNK_FLUSH_BEFORE_SLEEP_MS 1500UL

// Telegram Bot photo delivery
// 1. Create a bot with @BotFather and paste the token here.
// 2. Send a message to the bot, then read chat_id from:
//    https://api.telegram.org/bot<token>/getUpdates
#define TELEGRAM_BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define TELEGRAM_CHAT_ID "YOUR_TELEGRAM_CHAT_ID"
#define TELEGRAM_TIMEOUT_MS 30000UL
#define TELEGRAM_ALLOW_INSECURE_TLS 1
#define TELEGRAM_MAX_PHOTO_BYTES 250000UL

// Power behavior
#define SLEEP_AFTER_TASK_SEC 5UL
#define PERIODIC_WAKE_INTERVAL_SEC 21600ULL
#define SLEEP_WHEN_IDLE true
#define EXTERNAL_POWER_CONTROL_ENABLED 0
#define POWER_DONE_PIN 14
#define POWER_DONE_SIGNAL_MS 250UL
#define DEEP_SLEEP_EXT_WAKEUP_ENABLED 0

// Optional RTC time-window guard. Enable when DS3231 or PCF8523 is connected.
// This keeps the camera from doing Wi-Fi/camera work outside the active window.
#define RTC_ENABLED 0
#define RTC_TYPE_DS3231 1
#define RTC_TYPE_PCF8523 2
#define RTC_TYPE RTC_TYPE_DS3231
#define RTC_SDA_PIN 15
#define RTC_SCL_PIN 13
#define ACTIVE_WINDOW_ENABLED 0
#define RTC_REQUIRED_FOR_ACTIVE_WINDOW 1
#define ACTIVE_START_HOUR 8
#define ACTIVE_END_HOUR 16

// Capture behavior
#define CAPTURE_RETRY_COUNT 3
#define CAMERA_JPEG_QUALITY 12
#define CAMERA_FRAME_SIZE FRAMESIZE_XGA

// Pins for ESP32-CAM AI Thinker
#define FLASH_LED_PIN 4
#define WAKEUP_PIN GPIO_NUM_13
#define BATTERY_ADC_PIN 33
#define BATTERY_ADC_ENABLED 0
#define BATTERY_DIVIDER_RATIO 2.0f
#define LOW_BATTERY_VOLTAGE 3.45f
