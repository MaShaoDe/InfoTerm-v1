// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 FerienBSD aka Marcel Sauder
//
// WLAN (bis zu 3 Netze, der Reihe nach). Zugaenge: siehe settings.h (/secrets.txt).
// Das Passwort wird nie ausgegeben. Kein Server, keine Zeit, kein HTTPS im Prototyp.

#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "settings.h"

static bool netConnect(uint32_t timeoutMsPerNet) {
  if (cfg.wifiCount == 0) {
    Serial.println("WLAN: keine Zugangsdaten in /secrets.txt");
    return false;
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  for (int i = 0; i < cfg.wifiCount; i++) {
    Serial.printf("WLAN: verbinde mit %s\n", cfg.wifiSsid[i].c_str());
    WiFi.begin(cfg.wifiSsid[i].c_str(), cfg.wifiPass[i].c_str());
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < timeoutMsPerNet) {
      delay(100);
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("WLAN: verbunden nach %u ms, Signal %d dBm\n", (unsigned)(millis() - t0), (int)WiFi.RSSI());
      return true;
    }
    WiFi.disconnect();
    Serial.println("WLAN: nicht verbunden");
  }
  return false;
}
