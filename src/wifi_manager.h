#pragma once

#include <Arduino.h>

class WifiManager {
 public:
  bool connect();
  bool isConnected() const;
  int rssi() const;
};
