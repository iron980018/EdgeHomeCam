#include <Arduino.h>
#include <time.h>

#include "app_state.h"
#include "blynk_service.h"
#include "camera_manager.h"
#include "config.h"
#include "power_manager.h"
#include "upload_service.h"
#include "wifi_manager.h"

CameraManager camera;
WifiManager wifi;
PowerManager power;
UploadService uploader;

volatile bool captureRequested = false;
volatile bool sleepRequested = false;
unsigned long commandWindowStartedAt = 0;
bool cameraReady = false;

void requestCapture() {
  captureRequested = true;
}

void requestSleep() {
  sleepRequested = true;
}

void requestReboot() {
  ESP.restart();
}

void setFlash(bool enabled) {
  digitalWrite(FLASH_LED_PIN, enabled ? HIGH : LOW);
}

String isoTimestamp() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 500)) {
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%z", &timeinfo);
    return String(buffer);
  }

  return String("uptime-") + millis();
}

void configureTime() {
  configTzTime("CST-8", "pool.ntp.org", "time.nist.gov");
}

void publishStatus(DeviceStatus status) {
  Serial.printf("Status: %s\n", statusToString(status));
  BlynkApp.publishStatus(status);
}

CaptureResult captureAndUpload() {
  CaptureResult result;
  publishStatus(DeviceStatus::Capturing);

  if (!cameraReady) {
    result.errorCode = "CAMERA_INIT_FAILED";
    return result;
  }

  setFlash(true);
  delay(500);
  camera_fb_t *frame = camera.capture();
  setFlash(false);

  if (!frame) {
    result.errorCode = "CAPTURE_FAILED";
    return result;
  }

  String capturedAt = isoTimestamp();
  publishStatus(DeviceStatus::Uploading);

  String imageUrl;
  String errorCode;
  bool uploaded = uploader.upload(frame, capturedAt, power.batteryVoltage(), wifi.rssi(),
                                  imageUrl, errorCode);
  camera.release(frame);

  result.success = uploaded;
  result.imageUrl = imageUrl;
  result.errorCode = uploaded ? "" : errorCode;
  BlynkApp.publishCaptureResult(result, capturedAt);
  return result;
}

void goToSleepSoon() {
  publishStatus(DeviceStatus::Sleeping);
  BlynkApp.runFor(BLYNK_FLUSH_BEFORE_SLEEP_MS);
  delay(SLEEP_AFTER_TASK_SEC * 1000UL);
  power.sleepNow(PERIODIC_WAKE_INTERVAL_SEC);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("EdgeCam booting");

  power.begin();
  BlynkApp.setCallbacks(requestCapture, requestSleep, requestReboot, setFlash);

  if (power.isLowBattery()) {
    Serial.println("Low battery, sleeping");
    power.sleepNow(PERIODIC_WAKE_INTERVAL_SEC);
  }

  publishStatus(DeviceStatus::Booting);
  publishStatus(DeviceStatus::Connecting);

  if (!wifi.connect()) {
    Serial.println("Wi-Fi timeout");
    power.sleepNow(PERIODIC_WAKE_INTERVAL_SEC);
  }

  configureTime();
  cameraReady = camera.begin();

  if (!BlynkApp.begin()) {
    Serial.println("Blynk timeout");
  }

  BlynkApp.syncNeedPicture();
  BlynkApp.runFor(BLYNK_SYNC_TIMEOUT_MS);
  BlynkApp.publishTelemetry(power.batteryVoltage(), wifi.rssi());

  if (!cameraReady) {
    BlynkApp.publishError("CAMERA_INIT_FAILED");
  }

  if (AUTO_CAPTURE_ON_EXTERNAL_WAKE && power.wokeFromExternalPin()) {
    captureRequested = true;
  }

  commandWindowStartedAt = millis();
  publishStatus(DeviceStatus::Idle);
}

void loop() {
  BlynkApp.run();

  if (captureRequested) {
    captureRequested = false;
    CaptureResult result = captureAndUpload();
    if (!result.success) {
      Serial.printf("Telegram delivery failed: %s\n", result.errorCode.c_str());
    } else {
      BlynkApp.clearNeedPicture();
      BlynkApp.runFor(BLYNK_FLUSH_BEFORE_SLEEP_MS);
    }

    if (SLEEP_WHEN_IDLE) {
      goToSleepSoon();
    } else {
      publishStatus(DeviceStatus::Idle);
    }
  }

  if (sleepRequested) {
    sleepRequested = false;
    goToSleepSoon();
  }

  if (SLEEP_WHEN_IDLE && millis() - commandWindowStartedAt > COMMAND_WINDOW_MS) {
    goToSleepSoon();
  }

  delay(BLYNK_RUN_INTERVAL_MS);
}
