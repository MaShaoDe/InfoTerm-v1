<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Copyright (C) 2026 FerienBSD aka Marcel Sauder -->

# Changelog

## 0.1.0 (Vorschlag, offen: nicht kompiliert, nicht am Gerät getestet)

- Erste neutrale Fassung, herausgelöst aus der Arbeitszeit Terminal Firmware 0.11.0.
- Eingabe von ID (4 Stellen) und PIN (6 bis 8 Stellen) am Touch Display, ohne Prüfung und ohne Server.
- Infoseite mit Version, Name, WLAN, IP Adresse und freiem Speicher bei 4 Sekunden Druck aufs Display.
- Web Menü (WLAN, Anzeige, Gerätename, Passwort), WLAN aus `secrets.txt`.
- Ton über PNP Transistor, Bildschirmschoner, Touch Kalibrierung.
- Weggelassen: Sprachwahl, Flaggen, Bitmaps, Server Verbindung, Zertifikat, NTP, Sperre.
