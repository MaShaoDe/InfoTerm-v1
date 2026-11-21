<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Copyright (C) 2026 FerienBSD aka Marcel Sauder -->

# InfoTerm v1

Kleines Touch Terminal mit Zehnerblock: D1 mini (ESP8266), 2,8 Zoll Display mit Touch, Summer. Der Prototyp nimmt eine ID (4 Stellen) und eine PIN (6 bis 8 Stellen) entgegen, prüft nichts und spricht mit keinem Server. Er ist die neutrale Basis für Terminals aller Art (Herkunft: Arbeitszeit Terminal Firmware 0.11.0).

## Stand

Firmware 0.1.0 (Vorschlag): nicht kompiliert und nicht am Gerät getestet. Details: `STATUS.md`, `CHANGELOG.md`.

## Bedienung

- ID mit 4 Stellen eintippen, OK. PIN mit 6 bis 8 Stellen eintippen, OK. C löscht die letzte Ziffer, bei leerer PIN geht C zurück zur ID.
- **4 Sekunden aufs Display drücken**: Infoseite mit Version, Gerätename, WLAN, Signal, IP Adresse und freiem Speicher. Sie schliesst nach 15 Sekunden oder bei Berührung. Eine angefangene Eingabe wird dabei verworfen.
- Web Menü: IP Adresse der Infoseite im Browser öffnen (Benutzer `admin`, Passwort `gui_pass` aus `secrets.txt`, mindestens 8 Zeichen). Einstellbar: bis zu 3 WLAN Netze, Zeit bis Display aus, Gerätename, Passwort der Seite. Nach 5 falschen Eingaben ist die Seite bis zum Neustart gesperrt.
- Display aus nach 180 Sekunden ohne Eingabe (in `platformio.ini` für den Tischtest auf 20 gesetzt, im Web Menü einstellbar). Die weckende Berührung zählt nicht als Tastendruck.

## Hardware

Pinbelegung, Summer mit BC557 und 1,5 kΩ, Stückliste und offene Punkte für die Fertigung: `hardware/Spezifikation.md`.

## Bauen am Mac

```
cd ~/InfoTerm-v1
cp secrets.example.txt data/secrets.txt
open -e data/secrets.txt
pio run -t uploadfs -e d1_mini --upload-port /dev/cu.usbserial-A5069RR4
pio run -t upload -e d1_mini --upload-port /dev/cu.usbserial-A5069RR4
pio device monitor -b 115200 -p /dev/cu.usbserial-A5069RR4
```

`uploadfs` löscht den Speicher des Geräts, die Touch Kalibrierung (4 Ecken antippen) läuft danach einmal neu. Der D1 mini kann nur 2,4 GHz WLAN. Die USB Schnittstelle (`/dev/cu.usbserial-...`) mit `ls /dev/cu.*` prüfen.

## Lizenz

Firmware GPL-3.0-only, Hardware CERN-OHL-P-2.0, Dokumentation CC-BY-4.0. Copyright FerienBSD aka Marcel Sauder. Die Lizenztexte unter `LICENSES/` fehlen noch (offen).
