// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 FerienBSD aka Marcel Sauder
//
// Web Menue zum Einstellen des Geraets. Aufbau nach dem
// Web Menue der Bandampel (Reiter, WLAN Suche, Passwortfelder "leer = unveraendert",
// Speichern mit Neustart), angepasst an den ESP8266 und an die Sicherheit:
//   - Passwort der Seite ist Pflicht (Benutzer admin), nach 5 Fehlversuchen gesperrt
//   - gespeicherte Passwoerter werden nie angezeigt

#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include "settings.h"

static ESP8266WebServer web(80);
static uint32_t webRestartAt = 0;
static uint32_t webActivity = 0;  // millis() der letzten Anfrage
static uint8_t webAuthFails = 0;
static const uint8_t WEB_MAX_FAILS = 5;

static String he(const String &s) {  // HTML maskieren
  String o;
  for (unsigned i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '&') o += "&amp;";
    else if (c == '<') o += "&lt;";
    else if (c == '>') o += "&gt;";
    else if (c == '"') o += "&quot;";
    else o += c;
  }
  return o;
}

static bool webAuth() {
  webActivity = millis();
  if (webAuthFails >= WEB_MAX_FAILS) {
    web.send(429, "text/plain; charset=utf-8", "Gesperrt nach zu vielen Fehlversuchen. Geraet neu starten.");
    return false;
  }
  if (cfg.guiPass.length() >= 8 && web.authenticate("admin", cfg.guiPass.c_str())) {
    webAuthFails = 0;
    return true;
  }
  if (web.hasHeader("Authorization")) webAuthFails++;  // falsche Eingabe, nicht erste Anfrage
  web.requestAuthentication(BASIC_AUTH, "InfoTerm");
  return false;
}

static String fInput(const String &label, const String &name, const String &val, const char *type = "text",
                     const char *extra = "") {
  return "<label>" + label + "<input type=" + type + " name=" + name + " value=\"" + he(val) + "\" " + extra +
         "></label>";
}

static String kv(const char *k, const String &v) {
  return String("<div class=kv><span>") + k + "</span><b>" + he(v) + "</b></div>";
}

static void webRoot() {
  if (!webAuth()) return;
  String h;
  h.reserve(6000);
  h += F("<!DOCTYPE html><html lang=de><head><meta charset=utf-8>"
         "<meta name=viewport content='width=device-width,initial-scale=1'><title>InfoTerm</title><style>"
         "*{box-sizing:border-box}body{margin:0;font:16px/1.4 system-ui,sans-serif;background:#0b0f18;color:#e8ecf4;padding-bottom:84px}"
         "header{padding:12px 14px 4px}h1{font-size:20px;margin:0}header small{color:#8a94a8}"
         "nav{position:sticky;top:0;z-index:5;display:flex;flex-wrap:wrap;gap:6px;padding:8px 10px;background:#0b0f18;border-bottom:1px solid #243049}"
         "nav button{flex:1 1 auto;min-height:44px;padding:8px 12px;border:1px solid #243049;border-radius:10px;background:#151d2e;color:#cfd6e6;font-size:15px}"
         "nav button.on{background:#fc0;color:#111;border-color:#fc0;font-weight:600}"
         "main{max-width:720px;margin:0 auto;padding:10px}.tab{display:none}"
         ".card{background:#151d2e;border:1px solid #243049;border-radius:12px;padding:12px;margin:10px 0}"
         "h2{font-size:17px;margin:0 0 8px}label{display:block;margin:8px 0;color:#cfd6e6}"
         "input{width:100%;padding:10px;border-radius:8px;border:1px solid #243049;background:#0b0f18;color:#e8ecf4;font-size:16px}"
         "small{color:#8a94a8;display:block;margin-top:6px}.kv{display:flex;justify-content:space-between;padding:4px 0;border-bottom:1px solid #243049}"
         ".kv span{color:#8a94a8}.bar{position:fixed;left:0;right:0;bottom:0;padding:10px;background:#0b0f18;border-top:1px solid #243049}"
         ".b{width:100%;min-height:48px;border-radius:10px;border:0;font-size:17px;font-weight:600;background:#fc0;color:#111}"
         ".b.g{background:#151d2e;color:#cfd6e6;border:1px solid #243049;min-height:40px;font-size:15px}"
         "a{color:#fc0}</style></head><body><header><h1>InfoTerm</h1><small>Konfiguration, Version " FW_VERSION "</small></header>"
         "<nav><button type=button>WLAN</button><button type=button>Anzeige</button><button type=button>Gerät</button>"
         "<button type=button>Zugang</button><button type=button>Info</button></nav>"
         "<main><form id=f method=post action=/save autocomplete=off>");

  // 0 WLAN
  h += F("<section class=tab><div class=card><h2>WLAN Netze</h2>"
         "<small>Bis zu 3 Netze, das Gerät probiert sie der Reihe nach. Nur 2,4 GHz. Passwortfeld leer lassen = unverändert. "
         "Netzname leer = Eintrag löschen.</small>");
  h += F("<button type=button class='b g' onclick=scan()>Netze suchen</button><small id=sc></small><datalist id=nets></datalist>");
  for (int i = 0; i < CFG_MAX_WIFI; i++) {
    h += "<label>Netz " + String(i + 1) + "<input name=ws" + i + " list=nets value=\"" + he(cfg.wifiSsid[i]) + "\"></label>";
    h += fInput("Passwort " + String(i + 1) + (cfg.wifiSsid[i].length() ? " (gesetzt)" : ""), "wp" + String(i), "", "password",
                "autocomplete=new-password");
  }
  h += F("</div></section>");

  // 1 Anzeige
  h += F("<section class=tab><div class=card><h2>Anzeige</h2>");
  h += fInput("Display aus nach (Sekunden ohne Eingabe, 10 bis 3600)", "sav", String(cfg.saverS), "number", "min=10 max=3600");
  h += F("</div></section>");

  // 2 Geraet
  h += F("<section class=tab><div class=card><h2>Gerät</h2>");
  h += fInput("Gerätename", "did", cfg.deviceId, "text", "maxlength=32");
  h += F("</div></section>");

  // 3 Zugang
  h += F("<section class=tab><div class=card><h2>Passwort dieser Seite</h2>");
  h += fInput("Neues Passwort (mindestens 8 Zeichen, leer = unverändert)", "gp", "", "password", "autocomplete=new-password maxlength=32");
  h += F("<small>Benutzername im Browser: admin. Nach 5 falschen Eingaben ist die Seite bis zum Neustart gesperrt. "
         "Die Seite erreicht man über die IP Adresse des Geräts (Anzeige: 4 Sekunden aufs Display drücken).</small></div></section>");

  // 4 Info
  h += F("<section class=tab><div class=card><h2>Info</h2>");
  h += kv("Version", FW_VERSION) + kv("Freier Speicher", String(ESP.getFreeHeap()) + " Byte");
  h += F("<small><a href=/touch.cal>Touch Kalibrierung herunterladen</a>. Speichere die Datei als <b>data/touch.cal</b> im Ordner der "
         "Firmware, dann bleibt die Kalibrierung beim Aufspielen der Dateien erhalten.</small></div>"
         "<div class=card><h2>Lizenz</h2>");
  h += kv("Firmware", "GPL-3.0-only") + kv("Urheber", "FerienBSD aka Marcel Sauder");
  h += F("</div></section>");

  h += F("</form></main><div class=bar><button type=submit form=f class=b>Speichern und neu starten</button></div>"
         "<script>function T(i){var t=document.querySelectorAll('.tab'),b=document.querySelectorAll('nav button');"
         "for(var k=0;k<t.length;k++){t[k].style.display=k==i?'block':'none';b[k].className=k==i?'on':''}try{history.replaceState(0,'','#'+i)}catch(e){}}"
         "document.querySelectorAll('nav button').forEach(function(b,i){b.onclick=function(){T(i)}});"
         "function scan(){var s=document.getElementById('sc');s.textContent='Suche ...';fetch('/scan').then(function(r){return r.text()}).then(function(x){"
         "var d=document.getElementById('nets');d.innerHTML='';var n=0;x.split('\\n').forEach(function(l){if(!l)return;var p=l.split('\\t');"
         "var o=document.createElement('option');o.value=p[0];o.label=p[0]+' ('+p[1]+' dBm)';d.appendChild(o);n++});"
         "s.textContent=n+' Netze gefunden.'}).catch(function(){s.textContent='Suche fehlgeschlagen'})}"
         "T(parseInt((location.hash||'#0').slice(1))||0)</script></body></html>");
  web.send(200, "text/html; charset=utf-8", h);
}

static void webScan() {
  if (!webAuth()) return;
  int n = WiFi.scanNetworks();
  String out;
  for (int i = 0; i < n; i++) {
    String s = WiFi.SSID(i);
    if (!s.length()) continue;
    int r = WiFi.RSSI(i);
    bool skip = false;
    for (int j = 0; j < n; j++) {
      if (j == i || WiFi.SSID(j) != s) continue;
      if (WiFi.RSSI(j) > r || (WiFi.RSSI(j) == r && j < i)) {
        skip = true;  // nur das staerkste je Name
        break;
      }
    }
    if (skip) continue;
    out += s + "\t" + String(r) + "\n";
  }
  WiFi.scanDelete();
  web.send(200, "text/plain; charset=utf-8", out);
}

static String cleanId(const String &in) {
  String o;
  for (unsigned i = 0; i < in.length() && o.length() < 32; i++) {
    char c = in[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_') o += c;
  }
  return o;
}

static void webSave() {
  if (!webAuth()) return;
  String gp = web.arg("gp");
  if (gp.length() > 0 && gp.length() < 8) {
    web.send(400, "text/plain; charset=utf-8", "Das neue Passwort braucht mindestens 8 Zeichen. Nichts gespeichert.");
    return;
  }
  // WLAN Plaetze: leeres Passwort = unveraendert, wenn der Netzname gleich bleibt.
  String oldS[CFG_MAX_WIFI], oldP[CFG_MAX_WIFI];
  for (int i = 0; i < CFG_MAX_WIFI; i++) {
    oldS[i] = cfg.wifiSsid[i];
    oldP[i] = cfg.wifiPass[i];
  }
  for (int i = 0; i < CFG_MAX_WIFI; i++) {
    String s = web.arg("ws" + String(i));
    s.trim();
    String p = web.arg("wp" + String(i));
    if (s.length() && !p.length()) {
      for (int k = 0; k < CFG_MAX_WIFI; k++) {
        if (oldS[k] == s) p = oldP[k];
      }
    }
    cfg.wifiSsid[i] = s.substring(0, 32);
    cfg.wifiPass[i] = p.substring(0, 63);
  }
  long v = web.arg("sav").toInt();
  if (v > 0) cfg.saverS = (uint32_t)v;
  cfg.deviceId = cleanId(web.arg("did"));
  if (gp.length()) cfg.guiPass = gp.substring(0, 32);
  if (!cfgSave()) {
    web.send(500, "text/plain; charset=utf-8", "Speichern fehlgeschlagen.");
    return;
  }
  web.sendHeader("Location", "/saved");  // Reload sendet das Formular nicht nochmals
  web.send(303, "text/plain", "");
  webRestartAt = millis() + 3000;
}

static void webSaved() {
  webActivity = millis();
  web.send(200, "text/html; charset=utf-8",
           F("<!DOCTYPE html><html lang=de><meta charset=utf-8><meta name=viewport content='width=device-width,initial-scale=1'>"
             "<body style='font:18px system-ui;background:#0b0f18;color:#e8ecf4;padding:24px'>"
             "<h2>Gespeichert</h2><p>Das Gerät startet neu. Diese Seite schliesst sich, das WLAN des Geräts verschwindet.</p></body></html>"));
}

static void webCal() {
  if (!webAuth()) return;
  File f = LittleFS.open("/touch.cal", "r");
  if (!f) {
    web.send(404, "text/plain; charset=utf-8", "Keine Kalibrierung gespeichert.");
    return;
  }
  web.sendHeader("Content-Disposition", "attachment; filename=touch.cal");
  web.streamFile(f, "application/octet-stream");
  f.close();
}

static void webInit() {
  web.on("/saved", HTTP_GET, webSaved);
  web.on("/scan", HTTP_GET, webScan);
  web.on("/touch.cal", HTTP_GET, webCal);
  web.on("/", HTTP_GET, webRoot);
  web.on("/save", HTTP_POST, webSave);
  web.onNotFound([]() {
    web.sendHeader("Location", "/");
    web.send(302, "text/plain", "");
  });
  web.begin();
  webActivity = millis();
}

// Im Konfigurationsmodus laufend aufrufen. Nach dem Speichern startet das Geraet neu.
static void webLoop() {
  web.handleClient();
  if (webRestartAt && (int32_t)(millis() - webRestartAt) >= 0) ESP.restart();
}
