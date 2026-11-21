// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 FerienBSD aka Marcel Sauder
//
// Einstellungen des InfoTerm. Zwei Dateien im Geraet (LittleFS):
//   /config.txt   ohne Geheimnisse (im Repository, Ordner data/)
//   /secrets.txt  WLAN Zugaenge und Passwort der Konfigurationsseite
//                 (nie im Repository, Vorlage secrets.example.txt)
// Eine Zeile je Einstellung: name=wert. Das Web Menue (webcfg.h) schreibt beide Dateien.

#pragma once
#include <Arduino.h>
#include <LittleFS.h>

#define CFG_MAX_WIFI 3

// Bildschirmschoner: nach dieser Zeit ohne Eingabe ist das Display aus.
// Standard 3 Minuten, platformio.ini kann fuer den Tischtest einen anderen Wert setzen.
#ifndef SAVER_SECONDS
#define SAVER_SECONDS 180
#endif

struct Cfg {
  String wifiSsid[CFG_MAX_WIFI];
  String wifiPass[CFG_MAX_WIFI];
  int wifiCount;
  String guiPass;    // Passwort der Konfigurationsseite (Benutzer: admin), mindestens 8 Zeichen
  String deviceId;   // Name dieses Geraets
  uint32_t saverS;   // Sekunden bis Display aus
};

static Cfg cfg;

static void cfgDefaults() {
  cfg.wifiCount = 0;
  for (int i = 0; i < CFG_MAX_WIFI; i++) {
    cfg.wifiSsid[i] = "";
    cfg.wifiPass[i] = "";
  }
  cfg.guiPass = "";
  cfg.deviceId = "";
  cfg.saverS = SAVER_SECONDS;
}

static void cfgClamp() {
  if (cfg.saverS < 10) cfg.saverS = 10;
  if (cfg.saverS > 3600) cfg.saverS = 3600;
}

// Zaehlt die gefuellten WLAN Plaetze und schiebt sie nach vorn.
static void cfgPackWifi() {
  int n = 0;
  for (int i = 0; i < CFG_MAX_WIFI; i++) {
    if (cfg.wifiSsid[i].length() == 0) continue;
    if (n != i) {
      cfg.wifiSsid[n] = cfg.wifiSsid[i];
      cfg.wifiPass[n] = cfg.wifiPass[i];
    }
    n++;
  }
  for (int i = n; i < CFG_MAX_WIFI; i++) {
    cfg.wifiSsid[i] = "";
    cfg.wifiPass[i] = "";
  }
  cfg.wifiCount = n;
}

static void cfgReadFile(const char *name) {
  File f = LittleFS.open(name, "r");
  if (!f) {
    Serial.printf("Datei fehlt: %s\n", name);
    return;
  }
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0 || line.startsWith("#")) continue;
    int eq = line.indexOf('=');
    if (eq < 1) continue;
    String k = line.substring(0, eq);
    String v = line.substring(eq + 1);
    k.trim();
    v.trim();
    if (k.equals("wifi1_ssid")) cfg.wifiSsid[0] = v;
    else if (k.equals("wifi1_pass")) cfg.wifiPass[0] = v;
    else if (k.equals("wifi2_ssid")) cfg.wifiSsid[1] = v;
    else if (k.equals("wifi2_pass")) cfg.wifiPass[1] = v;
    else if (k.equals("wifi3_ssid")) cfg.wifiSsid[2] = v;
    else if (k.equals("wifi3_pass")) cfg.wifiPass[2] = v;
    else if (k.equals("gui_pass")) cfg.guiPass = v;
    else if (k.equals("device_id")) cfg.deviceId = v;
    else if (k.equals("saver_s")) cfg.saverS = (uint32_t)atol(v.c_str());
  }
  f.close();
}

static void cfgLoad() {
  cfgDefaults();
  cfgReadFile("/config.txt");
  cfgReadFile("/secrets.txt");
  cfgPackWifi();
  cfgClamp();
}

static bool cfgWriteFile(const char *name, const String &content) {
  File f = LittleFS.open(name, "w");
  if (!f) return false;
  size_t n = f.print(content);
  f.close();
  return n == content.length();
}

// Schreibt beide Dateien aus dem Speicher.
static bool cfgSave() {
  cfgPackWifi();
  cfgClamp();
  String c;
  c += "# Einstellungen ohne Geheimnisse (SPDX-License-Identifier: GPL-3.0-only)\n";
  c += "# Eine Zeile je Einstellung: name=wert\n";
  c += "device_id=" + cfg.deviceId + "\n";
  c += "saver_s=" + String(cfg.saverS) + "\n";
  String s;
  s += "# Geheimnisse, nie ins Repository\n";
  for (int i = 0; i < CFG_MAX_WIFI; i++) {
    if (cfg.wifiSsid[i].length() == 0) continue;
    s += "wifi" + String(i + 1) + "_ssid=" + cfg.wifiSsid[i] + "\n";
    s += "wifi" + String(i + 1) + "_pass=" + cfg.wifiPass[i] + "\n";
  }
  s += "gui_pass=" + cfg.guiPass + "\n";
  bool ok1 = cfgWriteFile("/config.txt", c);
  bool ok2 = cfgWriteFile("/secrets.txt", s);
  return ok1 && ok2;
}
