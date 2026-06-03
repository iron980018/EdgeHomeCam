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
#define TELEGRAM_ALLOW_INSECURE_TLS true

// Power behavior
#define COMMAND_WINDOW_MS 120000UL
#define SLEEP_AFTER_TASK_SEC 5UL
#define PERIODIC_WAKE_INTERVAL_SEC 21600ULL
#define AUTO_CAPTURE_ON_EXTERNAL_WAKE true
#define SLEEP_WHEN_IDLE true

// Capture behavior
#define CAPTURE_RETRY_COUNT 3
#define CAMERA_JPEG_QUALITY 12
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA

// Pins for ESP32-CAM AI Thinker
#define FLASH_LED_PIN 4
#define WAKEUP_PIN GPIO_NUM_13
#define BATTERY_ADC_PIN 33
#define BATTERY_ADC_ENABLED false
#define BATTERY_DIVIDER_RATIO 2.0f
#define LOW_BATTERY_VOLTAGE 3.45f
