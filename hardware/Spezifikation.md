<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Copyright (C) 2026 FerienBSD aka Marcel Sauder -->

# InfoTerm v1: technische Spezifikation (Hardware)

Stand 21.11.2026. Quelle: Aufbau auf dem Breadboard (Tischtest Marcel Sauder, 20.11.2026) und Firmware 0.1.0. Alles, was nicht gemessen oder nicht bestätigt ist, steht als «offen».

## 1. Bauteile

| Teil | Typ | Anzahl | Stand |
|---|---|---|---|
| Mikrocontroller Board | D1 mini Klon mit ESP8266 (Reserve: ESP8684) | 1 | bestätigt |
| Display mit Touch | 2,8 Zoll TFT SPI, 240 x 320, Treiber ILI9341, Touch XPT2046 (Modul MSP2807) | 1 | bestätigt |
| Transistor | BC557 (PNP) | 1 | bestätigt (Marcel Sauder, 21.11.2026) |
| Basiswiderstand | 1,5 kΩ | 1 | bestätigt (Marcel Sauder, 21.11.2026) |
| Summer | SFM-20B, aktiver Piezo Summer, Dauerton, ohne Marke (siehe 3.1) | 1 | Daten nach Händlerangabe, Strom: offen |
| Stromversorgung | USB-C Buchse auf dem PCB (Vorschlag), 2 x 5,1 kΩ an CC1 und CC2, Polyfuse, 220 µF, 100 nF, speist den 5V Pin | 1 | Vorschlag, Strom: offen |

Nicht verwendet: externer Flash W25Q128 auf dem Display Modul, SD Slot.

## 2. Pinbelegung

| Display Modul | D1 mini | GPIO | Bemerkung |
|---|---|---|---|
| VCC | 3V3 | | Logikpegel 3,3 V |
| GND | G | | |
| CS | D8 | 15 | |
| RESET | RST | | |
| DC | D2 | 4 | |
| SDI (MOSI) | D7 | 13 | |
| SCK | D5 | 14 | |
| LED | D0 | 16 | Hintergrundlicht ein und aus. Strom am Pin nicht gemessen: offen |
| SDO (MISO) | unbeschaltet | | stört sonst den Touch |
| T_CLK | D5 | 14 | gleiche Leitung wie SCK |
| T_CS | D1 | 5 | |
| T_DIN | D7 | 13 | gleiche Leitung wie MOSI |
| T_DO | D6 | 12 | MISO |
| T_IRQ | unbeschaltet | | Touch wird abgefragt |

GPIO Nummern für CS, DC, T_CS, LED und Summer stehen in `platformio.ini` und `src/main.cpp`.

## 3. Summer mit PNP Transistor

| Von | Nach |
|---|---|
| D4 (GPIO2) | 1,5 kΩ, dann Basis BC557 |
| Emitter BC557 | 3V3 |
| Kollektor BC557 | Summer plus |
| Summer minus | G (GND) |

- Ton ein = D4 niedrig. D4 ist ein Bootpin und muss beim Start hoch sein. Der PNP sperrt bei hohem Pegel, der Summer ist beim Einschalten still.
- Ein NPN Transistor an D4 klemmt den Pin über die Basis nach unten, das Gerät startet dann in einer Schleife neu (Tischtest 20.11.2026 mit NPN). Darum PNP.
- Berechnung (nicht gemessen): Basisstrom etwa (3,3 V minus 0,7 V) / 1,5 kΩ = 1,7 mA. Mit der Faustregel «Basisstrom ein Zehntel des Kollektorstroms» trägt die Schaltung einen Summer bis etwa 17 mA. Der Strom des verwendeten Summers ist offen.
### 3.1 Summer SFM-20B (Händlerangaben, nicht gemessen)

| Angabe | Wert |
|---|---|
| Typ | aktiver Piezo Summer, Dauerton, ohne Marke |
| Grösse | 23 x 12 mm |
| Nennspannung | 12 V DC |
| Betriebsspannung | 3 bis 24 V DC |
| Arbeitsfrequenz | 3300 Hz, plus minus 500 Hz |
| Schalldruck | 90 dB (die Produktseite nennt im Titel 95 dB: widersprüchlich) |
| Arbeitstemperatur | minus 20 bis plus 80 Grad |
| Stromaufnahme | nicht angegeben: offen |

- Am D1 mini liegt der Summer an 3,3 V, abzüglich der Sättigungsspannung des BC557 sind es etwa 3,1 V, also knapp über der unteren Betriebsgrenze von 3 V (Berechnung, nicht gemessen). Der Ton ist dort leiser als die 90 dB bei Nennspannung.
- Mit 5 V am Emitter ginge der PNP bei hohem Pegel von 3,3 V nicht mehr sicher zu (Basis liegt 1,7 V unter dem Emitter). Für einen lauteren Summer wäre eine andere Schaltung nötig (offen).
- Lieferumfang der Bestellung: 5 Stück.

- Piept der Summer im Ruhezustand, `BUZZER_ON` in `src/main.cpp` auf `HIGH` setzen.
- Töne: jede Taste ein kurzer Klick, Eingabe komplett ein Ton (0,25 s).
- Hinweis: In der Arbeitszeit Firmware stand bisher 1 kΩ im README. Gebaut und getestet ist 1,5 kΩ.

## 4. Für die Fertigung (Vorschlag, offen)

- Platine mit Buchsenleisten für den D1 mini und das Display Modul (steckbar), BC557 und Widerstand bedrahtet oder SMD, Summer auf Pfostenstecker oder gelötet.
- Offen: Summer Stromaufnahme und Lautstärke bei 3,3 V, Hintergrundlicht Strom (D0 direkt oder über Transistor), Stromversorgung (Vorschlag: USB-C Buchse auf dem PCB, siehe Schaltplan), Gehäuse und Montage, Stückzahl, Fertiger, Schaltplan und Gerber Dateien (noch nicht gezeichnet).
- Vor der Bestellung ein Muster aufbauen und messen: Stromaufnahme mit und ohne Hintergrundlicht, Summerstrom, Temperatur im Dauerbetrieb.

## 5. Lizenz

Hardware CERN-OHL-P-2.0, Firmware GPL-3.0-only, Dokumentation CC-BY-4.0.
