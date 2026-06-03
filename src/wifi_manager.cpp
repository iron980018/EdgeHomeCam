#include "wifi_manager.h"

#include <WiFi.h>
#include "config.h"

bool WifiManager::connect() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(true);

  for (int attempt = 1; attempt <= WIFI_RETRY_COUNT; attempt++) {
    Serial.printf("Wi-Fi attempt %d/%d\n", attempt, WIFI_RETRY_COUNT);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startedAt = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startedAt < WIFI_CONNECT_TIMEOUT_MS) {
      delay(250);
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("Wi-Fi connected, IP: %s, RSSI: %d\n",
                    WiFi.localIP().toString().c_str(), WiFi.RSSI());
      return true;
    }

    WiFi.disconnect(true);
    delay(500);
  }

  return false;
}

bool WifiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

int WifiManager::rssi() const {
  return WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
}
