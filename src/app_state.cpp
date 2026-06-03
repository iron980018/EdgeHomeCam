#include "app_state.h"

const char *statusToString(DeviceStatus status) {
  switch (status) {
    case DeviceStatus::Booting:
      return "booting";
    case DeviceStatus::Connecting:
      return "connecting";
    case DeviceStatus::Idle:
      return "idle";
    case DeviceStatus::Capturing:
      return "capturing";
    case DeviceStatus::Uploading:
      return "uploading";
    case DeviceStatus::Sleeping:
      return "sleeping";
    case DeviceStatus::Error:
      return "error";
    default:
      return "unknown";
  }
}
