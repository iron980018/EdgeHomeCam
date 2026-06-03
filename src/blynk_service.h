#pragma once

#include <Arduino.h>
#include "app_state.h"

using CaptureCallback = void (*)();
using SleepCallback = void (*)();
using RebootCallback = void (*)();

class BlynkService {
 public:
  bool begin();
  void run();
  void runFor(unsigned long durationMs);
  bool connected() const;
  void syncNeedPicture();
  void clearNeedPicture();
  void setCallbacks(CaptureCallback capture, SleepCallback sleep, RebootCallback reboot);
  void publishStatus(DeviceStatus status);
  void publishTelemetry(float batteryVoltage, int rssi);
  void publishCaptureResult(const CaptureResult &result, const String &capturedAt);
  void publishError(const String &errorCode);
  void handleCaptureCommand();
  void handleSleepCommand();
  void handleRebootCommand();

 private:
  CaptureCallback captureCallback_ = nullptr;
  SleepCallback sleepCallback_ = nullptr;
  RebootCallback rebootCallback_ = nullptr;
};

extern BlynkService BlynkApp;
