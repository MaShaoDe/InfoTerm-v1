// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 FerienBSD aka Marcel Sauder
//
// InfoTerm v1, Prototyp: Eingabe von ID und PIN am Touch Display, sonst nichts.
// Schritt 1: ID eintippen (genau 4 Stellen), OK.
// Schritt 2: PIN eintippen (6 bis 8 Stellen), OK. Danach kurze Meldung, zurueck zum Start.
// Keine Pruefung, kein Server. Die PIN Ziffern werden nie ausgegeben.
// 4 Sekunden aufs Display druecken: Infoseite mit Version, WLAN, IP Adresse und Speicher.
// Web Menue (WLAN, Anzeige, Geraetename, Passwort): siehe webcfg.h.
// Pins und Aufbau: siehe README.md, hardware/ und platformio.ini.

#define FW_VERSION "0.1.0"

#include <Arduino.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include "settings.h"
#include "net.h"
#include "webcfg.h"

static const uint8_t PIN_BACKLIGHT = 16;  // D0, am LED Pin des Displays
// Pegel, bei dem das Hintergrundlicht an ist. Schaltet es verkehrt herum, hier LOW eintragen.
static const uint8_t BACKLIGHT_ON = HIGH;

// Ton: aktiver Summer ueber einen PNP Transistor (BC557, Plus-Seite) an D4 (GPIO2).
// D4 ist ein Bootpin und muss beim Start hoch sein. Der PNP sperrt bei hohem
// Pegel, der Summer ist beim Start still. Ton ein = D4 niedrig.
static const uint8_t PIN_BUZZER = 2;  // D4
static const uint8_t BUZZER_ON = LOW;

static const char *CAL_FILE = "/touch.cal";
static const uint8_t ID_LEN = 4;   // ID hat genau 4 Stellen
static const uint8_t PIN_MIN = 6;  // PIN hat 6 bis 8 Stellen
static const uint8_t PIN_MAX = 8;
static const uint32_t LONG_PRESS_MS = 4000;  // Infoseite: so lange das Display druecken
static const uint32_t INFO_SHOW_MS = 15000;  // Infoseite: so lange sichtbar (oder bis zur Beruehrung)

// Layout 240 x 320 (hochkant): oben Kopf (Titel, Aufforderung, Eingabe),
// darunter 3 Spalten x 4 Zeilen Zehnerblock.
static const int KEY_W = 80;
static const int KEY_H = 56;
static const int KEY_Y0 = 96;

static const char *LABELS[12] = {"1", "2", "3", "4", "5", "6",
                                 "7", "8", "9", "C", "0", "OK"};

enum Mode : uint8_t { MODE_ID, MODE_PIN };

TFT_eSPI tft = TFT_eSPI();
uint16_t calData[5];
Mode mode = MODE_ID;
char idBuf[ID_LEN + 1];
uint8_t idLen = 0;
char pinBuf[PIN_MAX + 1];
uint8_t pinLen = 0;
bool screenOn = true;
uint32_t lastActivity = 0;

bool loadCal() {
  if (!LittleFS.exists(CAL_FILE)) {
    return false;
  }
  File f = LittleFS.open(CAL_FILE, "r");
  if (!f || f.size() != sizeof(calData)) {
    return false;
  }
  f.read((uint8_t *)calData, sizeof(calData));
  f.close();
  return true;
}

void saveCal() {
  File f = LittleFS.open(CAL_FILE, "w");
  if (f) {
    f.write((uint8_t *)calData, sizeof(calData));
    f.close();
  }
}

void runCal() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Ecken antippen", 120, 150, 2);
  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  saveCal();
}

void drawKey(int i, bool pressed) {
  int col = i % 3;
  int row = i / 3;
  int x = col * KEY_W;
  int y = KEY_Y0 + row * KEY_H;
  uint16_t fill = pressed ? TFT_DARKGREEN : TFT_NAVY;
  if (i == 9) {
    fill = pressed ? TFT_MAROON : TFT_RED;
  }
  if (i == 11) {
    fill = pressed ? TFT_DARKGREEN : TFT_GREEN;
  }
  tft.fillRoundRect(x + 3, y + 3, KEY_W - 6, KEY_H - 6, 8, fill);
  tft.setTextColor(TFT_WHITE, fill);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(LABELS[i], x + KEY_W / 2, y + KEY_H / 2, 4);
}

// WLAN Symbol oben rechts: 4 Balken. Nicht verbunden: alle weiss.
// Verbunden: so viele gruene Balken wie die Signalstaerke hergibt (1 bis 4),
// der Rest dunkelgrau. Grenzen: ab -55 dBm 4, ab -65 dBm 3, ab -75 dBm 2.
static bool wifiUp = false;
static uint8_t wifiBars = 0;

void wifiPoll() {
  wifiUp = (WiFi.status() == WL_CONNECTED);
  if (!wifiUp) {
    wifiBars = 0;
    return;
  }
  int r = WiFi.RSSI();
  wifiBars = r >= -55 ? 4 : r >= -65 ? 3 : r >= -75 ? 2 : 1;
}

void drawWifi() {
  const int x0 = 218, base = 24;  // rechts oben, 4 Balken je 3 breit
  for (int i = 0; i < 4; i++) {
    int h = 5 + i * 4;
    uint16_t c = !wifiUp ? TFT_WHITE : (i < wifiBars ? TFT_GREEN : 0x4208);
    tft.fillRect(x0 + i * 5, base - h, 3, h, c);
  }
}

void drawTitle() {
  tft.fillRect(0, 0, 240, 34, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("InfoTerm", 120, 6, 4);
  drawWifi();
}

void drawPrompt() {
  tft.fillRect(0, 36, 240, 24, TFT_BLACK);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(mode == MODE_ID ? "ID eingeben (4 Stellen)" : "PIN eingeben (6 bis 8)", 120, 40, 2);
}

// Kopf neu zeichnen: Titel, Aufforderung, Eingabe (ID als Zahl, PIN als Punkte).
void drawHeader() {
  tft.fillRect(0, 0, 240, KEY_Y0, TFT_BLACK);
  drawTitle();
  drawPrompt();
  tft.setTextDatum(TC_DATUM);
  if (mode == MODE_ID) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(idBuf, 120, 62, 4);
  } else {
    // 8 Platz Punkte: gefuellt = eingegeben, hell = Pflicht (ab 6), dunkel = frei.
    for (uint8_t i = 0; i < PIN_MAX; i++) {
      int x = 36 + i * 24;
      if (i < pinLen) {
        tft.fillCircle(x, 76, 7, TFT_YELLOW);
      } else {
        tft.drawCircle(x, 76, 7, i < PIN_MIN ? TFT_LIGHTGREY : TFT_DARKGREY);
      }
    }
  }
}

// Ganzer Bildschirm neu: Kopf mit Zehnerblock.
void fullRedraw() {
  tft.fillScreen(TFT_BLACK);
  drawHeader();
  for (int i = 0; i < 12; i++) drawKey(i, false);
}

void resetInput() {
  idLen = 0;
  idBuf[0] = 0;
  pinLen = 0;
  memset(pinBuf, 0, sizeof(pinBuf));
  mode = MODE_ID;
}

// Ton ohne Blockieren: n Toene zu je onMs, dazwischen offMs Pause.
static uint8_t buzLeft = 0;
static bool buzState = false;
static uint16_t buzOnMs = 0, buzOffMs = 0;
static uint32_t buzNext = 0;

void buzzer(bool on) {
  digitalWrite(PIN_BUZZER, on ? BUZZER_ON : !BUZZER_ON);
  buzState = on;
}

void beep(uint8_t n, uint16_t onMs, uint16_t offMs) {
  buzLeft = n;
  buzOnMs = onMs;
  buzOffMs = offMs;
  buzzer(true);
  buzNext = millis() + onMs;
}

void buzzerPoll() {
  if (buzLeft == 0 || (int32_t)(millis() - buzNext) < 0) return;
  if (buzState) {
    buzzer(false);
    buzLeft--;
    buzNext = millis() + buzOffMs;
  } else {
    buzzer(true);
    buzNext = millis() + buzOnMs;
  }
}

static bool webOn = false;

// Wartet ms Millisekunden und bedient dabei Ton und Web Menue.
void waitMs(uint32_t ms) {
  uint32_t t = millis();
  while (millis() - t < ms) {
    buzzerPoll();
    if (webOn) webLoop();
    delay(1);
  }
}

void backlight(bool on) {
  digitalWrite(PIN_BACKLIGHT, on ? BACKLIGHT_ON : !BACKLIGHT_ON);
}

// Display einschalten (Beruehrung).
void screenWake() {
  backlight(true);
  screenOn = true;
  lastActivity = millis();
  Serial.println("Display ein");
}

// Display ausschalten. Eine angefangene Eingabe wird vorher geloescht, damit
// niemand eine halbe ID oder PIN stehen laesst.
void screenSleep() {
  resetInput();
  fullRedraw();
  backlight(false);
  screenOn = false;
  Serial.println("Display aus");
}

// Meldung auf dem ganzen Bildschirm, bis zu 3 Zeilen.
static void showMsg(uint16_t color, const char *l1, const char *l2 = "", const char *l3 = "") {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(color, TFT_BLACK);
  tft.drawString(l1, 120, 110, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(l2, 120, 160, 4);
  tft.drawString(l3, 120, 200, 2);
}

// Infoseite (4 Sekunden Druck aufs Display): Version, Name, WLAN, IP Adresse, Speicher.
// Schliesst nach INFO_SHOW_MS oder bei neuer Beruehrung.
static void showInfo() {
  Serial.println("Infoseite");
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("InfoTerm", 120, 8, 4);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(String("Version  ") + FW_VERSION, 10, 56, 2);
  tft.drawString(String("Name     ") + (cfg.deviceId.length() ? cfg.deviceId : String("(leer)")), 10, 80, 2);
  if (WiFi.status() == WL_CONNECTED) {
    tft.drawString(String("WLAN     ") + WiFi.SSID(), 10, 104, 2);
    tft.drawString(String("Signal   ") + WiFi.RSSI() + " dBm", 10, 128, 2);
    tft.drawString("IP Adresse", 10, 160, 2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(WiFi.localIP().toString(), 10, 182, 4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(webOn ? "Web Menue: Benutzer admin" : "Web Menue aus (gui_pass fehlt)", 10, 220, 2);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("WLAN     nicht verbunden", 10, 104, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }
  tft.drawString(String("Frei     ") + ESP.getFreeHeap() + " Byte", 10, 250, 2);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Beruehren zum Schliessen", 120, 294, 2);

  uint16_t x, y;
  uint32_t t0 = millis();
  while (tft.getTouch(&x, &y) && millis() - t0 < 5000) {  // erst loslassen
    buzzerPoll();
    if (webOn) webLoop();
    delay(10);
  }
  delay(200);
  t0 = millis();
  while (millis() - t0 < INFO_SHOW_MS) {
    buzzerPoll();
    if (webOn) webLoop();
    if (tft.getTouch(&x, &y)) break;
    delay(10);
  }
  while (tft.getTouch(&x, &y)) delay(10);  // loslassen, damit nichts getippt wird
  resetInput();
  fullRedraw();
  lastActivity = millis();
}

// ID und PIN sind komplett. Prototyp: keine Pruefung, nur eine Meldung.
void finishInput() {
  Serial.printf("Eingabe komplett: ID %s, PIN %u Stellen\n", idBuf, pinLen);
  showMsg(TFT_GREEN, "Eingabe komplett", "Prototyp", "keine Pruefung");
  beep(1, 250, 0);
  waitMs(2000);
  resetInput();
  fullRedraw();
  lastActivity = millis();
}

void handleKey(int i) {
  beep(1, 15, 0);  // Klick
  drawKey(i, true);
  waitMs(80);
  drawKey(i, false);

  const char *label = LABELS[i];
  bool isDigit = (i != 9 && i != 11);
  // Die PIN Ziffern werden nie ausgegeben.
  if (mode == MODE_PIN && isDigit) {
    Serial.println("Taste *");
  } else {
    Serial.printf("Taste %s\n", label);
  }

  if (mode == MODE_ID) {
    if (isDigit) {
      if (idLen < ID_LEN) {
        idBuf[idLen++] = label[0];
        idBuf[idLen] = 0;
      }
    } else if (i == 9) {  // C: letzte Ziffer der ID loeschen
      if (idLen > 0) idBuf[--idLen] = 0;
    } else if (i == 11 && idLen == ID_LEN) {  // OK bei 4 Stellen: weiter zur PIN
      Serial.printf("ID mit %u Stellen, weiter zur PIN\n", idLen);
      mode = MODE_PIN;
      pinLen = 0;
    }
  } else {
    if (isDigit) {
      if (pinLen < PIN_MAX) {
        pinBuf[pinLen++] = label[0];
        pinBuf[pinLen] = 0;
      }
    } else if (i == 11 && pinLen >= PIN_MIN) {  // OK ab 6 Stellen: fertig
      finishInput();
      return;
    } else if (i == 9) {  // C: letzte Ziffer der PIN loeschen, bei leerer PIN zurueck zur ID
      if (pinLen > 0) {
        pinBuf[--pinLen] = 0;
      } else {
        mode = MODE_ID;
      }
    }
  }
  drawHeader();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.printf("\nInfoTerm %s\n", FW_VERSION);
  Serial.printf("Freier Speicher: %u Byte\n", ESP.getFreeHeap());

  pinMode(PIN_BACKLIGHT, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  buzzer(false);
  backlight(true);
  LittleFS.begin();
  cfgLoad();
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  bool needCal = !loadCal();
  if (needCal) {
    runCal();  // erster Start ohne Kalibrierung
  }
  tft.setTouch(calData);

  resetInput();
  fullRedraw();
  // WLAN: blockiert bis etwa 10 Sekunden je Netz, das Display reagiert in dieser Zeit nicht.
  netConnect(10000);
  if (cfg.guiPass.length() < 8) {
    Serial.println("Web Menue aus: gui_pass in secrets.txt fehlt oder ist kuerzer als 8 Zeichen");
  } else if (WiFi.status() == WL_CONNECTED) {
    webInit();
    webOn = true;
    Serial.printf("Web Menue: http://%s (Benutzer admin)\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("Web Menue aus: kein WLAN");
  }
  wifiPoll();
  fullRedraw();
  lastActivity = millis();
  Serial.printf("Bereit, Display aus nach %u Sekunden\n", (unsigned)cfg.saverS);
}

void loop() {
  buzzerPoll();
  if (webOn) webLoop();
  // Nach der eingestellten Zeit ohne Eingabe das Display ausschalten.
  if (screenOn && millis() - lastActivity > cfg.saverS * 1000UL) {
    screenSleep();
  }

  static uint32_t lastWifi = 0;
  if (millis() - lastWifi >= 2000) {
    lastWifi = millis();
    wifiPoll();
  }

  // WLAN Symbol nachfuehren, wenn sich die Signalstaerke aendert.
  static uint8_t shownBars = 255;
  static bool shownUp = false;
  if (screenOn && (wifiBars != shownBars || wifiUp != shownUp)) {
    shownBars = wifiBars;
    shownUp = wifiUp;
    drawWifi();
  }

  static bool down = false;
  static uint32_t last = 0;
  static uint32_t downSince = 0;
  static bool longDone = false;
  uint16_t x, y;
  bool touched = tft.getTouch(&x, &y);
  if (touched && !down && millis() - last > 150) {
    down = true;
    last = millis();
    downSince = millis();
    longDone = false;
    if (!screenOn) {
      // Die weckende Beruehrung zaehlt nicht als Tastendruck.
      screenWake();
      longDone = true;  // und loest auch die Infoseite nicht aus
    } else {
      lastActivity = millis();
      if (y >= KEY_Y0) {
        int col = x / KEY_W;
        int row = (y - KEY_Y0) / KEY_H;
        if (col < 3 && row < 4) {
          handleKey(row * 3 + col);
        }
      }
    }
  } else if (touched && down && !longDone && millis() - downSince >= LONG_PRESS_MS) {
    // 4 Sekunden gedrueckt: Infoseite. Eine dabei angefangene Eingabe wird verworfen.
    longDone = true;
    showInfo();
  } else if (!touched) {
    down = false;
  }
}
