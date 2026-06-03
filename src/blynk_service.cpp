#include "blynk_service.h"

#include "config.h"
#include <BlynkSimpleEsp32.h>

BlynkService BlynkApp;

bool BlynkService::begin() {
  Blynk.config(BLYNK_AUTH_TOKEN);
  unsigned long startedAt = millis();
  while (!Blynk.connected() && millis() - startedAt < BLYNK_CONNECT_TIMEOUT_MS) {
    if (Blynk.connect(1000)) {
      break;
    }
    delay(100);
  }

  Serial.printf("Blynk connected: %s\n", Blynk.connected() ? "yes" : "no");
  return Blynk.connected();
}

void BlynkService::run() {
  if (Blynk.connected()) {
    Blynk.run();
  }
}

void BlynkService::runFor(unsigned long durationMs) {
  unsigned long startedAt = millis();
  while (Blynk.connected() && millis() - startedAt < durationMs) {
    Blynk.run();
    delay(BLYNK_RUN_INTERVAL_MS);
  }
}

bool BlynkService::connected() const {
  return Blynk.connected();
}

void BlynkService::syncNeedPicture() {
  if (Blynk.connected()) {
    Blynk.syncVirtual(V0);
  }
}

void BlynkService::clearNeedPicture() {
  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, 0);
  }
}

void BlynkService::setCallbacks(CaptureCallback capture, SleepCallback sleep,
                                RebootCallback reboot, FlashCallback flash) {
  captureCallback_ = capture;
  sleepCallback_ = sleep;
  rebootCallback_ = reboot;
  flashCallback_ = flash;
}

void BlynkService::publishStatus(DeviceStatus status) {
  if (Blynk.connected()) {
    Blynk.virtualWrite(V1, statusToString(status));
  }
}

void BlynkService::publishTelemetry(float batteryVoltage, int rssi) {
  if (!Blynk.connected()) {
    return;
  }
  Blynk.virtualWrite(V4, batteryVoltage);
  Blynk.virtualWrite(V5, rssi);
}

void BlynkService::publishCaptureResult(const CaptureResult &result, const String &capturedAt) {
  if (!Blynk.connected()) {
    return;
  }

  Blynk.virtualWrite(V2, capturedAt);
  Blynk.virtualWrite(V3, result.imageUrl);
  Blynk.virtualWrite(V6, result.success ? "" : result.errorCode);
  Blynk.virtualWrite(V1, result.success ? "idle" : "error");
}

void BlynkService::publishError(const String &errorCode) {
  if (Blynk.connected()) {
    Blynk.virtualWrite(V1, "error");
    Blynk.virtualWrite(V6, errorCode);
  }
}

void BlynkService::handleCaptureCommand() {
  if (captureCallback_) {
    captureCallback_();
  }
}

void BlynkService::handleSleepCommand() {
  if (sleepCallback_) {
    sleepCallback_();
  }
}

void BlynkService::handleRebootCommand() {
  if (rebootCallback_) {
    rebootCallback_();
  }
}

void BlynkService::handleFlashCommand(bool enabled) {
  if (flashCallback_) {
    flashCallback_(enabled);
  }
}

BLYNK_WRITE(V0) {
  if (param.asInt() == 1) {
    BlynkApp.handleCaptureCommand();
  }
}

BLYNK_WRITE(V7) {
  if (param.asInt() == 1) {
    BlynkApp.handleSleepCommand();
  }
}

BLYNK_WRITE(V8) {
  BlynkApp.handleFlashCommand(param.asInt() == 1);
}

BLYNK_WRITE(V9) {
  if (param.asInt() == 1) {
    BlynkApp.handleRebootCommand();
  }
}
