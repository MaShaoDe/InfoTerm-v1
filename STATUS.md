# Status InfoTerm v1

Höchstens 30 Zeilen. Immer überschreiben, nicht anhängen. Erledigtes streichen.

## Stand
- Firmware 0.1.0 (Vorschlag): neutraler Prototyp aus der Arbeitszeit Terminal Firmware 0.11.0. ID und PIN am Touch Display, Infoseite bei 4 Sekunden Druck, Web Menü, WLAN aus `secrets.txt`. Nicht kompiliert, nicht am Gerät getestet.
- Hardware Spezifikation in `hardware/Spezifikation.md`: D1 mini Klon, Display MSP2807 (ILI9341, XPT2046), Summer über BC557 mit 1,5 kΩ an D4.

## Offen
- Summer: Typ, Nennspannung, Strom. Hintergrundlicht: Strom am Pin D0.
- Stromversorgung, Gehäuse, Schaltplan, Gerber Dateien, Fertiger und Stückzahl.
- Lizenztexte (GPL-3.0, CERN-OHL-P-2.0, CC-BY-4.0) unter `LICENSES/` fehlen noch.
- Test am Gerät: Kompilieren, Infoseite, Web Menü.
