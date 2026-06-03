#pragma once

#include <Arduino.h>

class RtcManager {
 public:
  bool begin();
  bool isReady() const;
  bool hasValidTime() const;
  bool isWithinActiveWindow() const;
  String nowString() const;

 private:
  bool ready_ = false;
  bool validTime_ = false;
};
