#pragma once

#include <Arduino.h>

enum class DeviceStatus {
  Booting,
  Connecting,
  Idle,
  Capturing,
  Uploading,
  Sleeping,
  Error
};

struct CaptureResult {
  bool success = false;
  String imageUrl;
  String errorCode;
};

const char *statusToString(DeviceStatus status);
