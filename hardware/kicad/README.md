<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Copyright (C) 2026 FerienBSD aka Marcel Sauder -->

# InfoTerm v1: Schaltplan (KiCad)

Stand 24.11.2026. Vorschlag, offen: `infoterm-v1.kicad_sch` wurde als Text geschrieben und nie in KiCad geöffnet oder geprüft (kein ERC). Beim Öffnen können Fehler auftreten. Schaltung nach `hardware/Spezifikation.md`.

Hardware: CERN-OHL-P-2.0.

## Bauteile (Vorschlag, Muster ab 5 Stück)

| Ref | Teil | Footprint (Vorschlag) |
|---|---|---|
| J1, J2 | Buchsenleiste 1x8, 2,54 mm, für D1 mini (Reihenabstand 22,86 mm: offen, am Board messen) | PinSocket_1x08 |
| J3 | Buchsenleiste 1x14 für Display MSP2807 | PinSocket_1x14 |
| Q1 | BC557 (PNP), Beinchen C, B, E im Datenblatt prüfen | TO-92 |
| R1 | 1,5 kΩ | Axial DIN0207 |
| J4 | Pfostenstecker 1x2 für Summer | PinHeader_1x02 |
| J5 | USB-C Buchse, nur Strom (16 Pin SMD, z.B. HRO TYPE-C-31-M-12), am Gehäuserand | USB_C_Receptacle_HRO_TYPE-C-31-M-12 |
| R2, R3 | 5,1 kΩ an CC1 und CC2 gegen GND (nötig, sonst liefert ein USB-C Netzteil keine 5 V) | R_0805 |
| F1 | rückstellende Sicherung (Polyfuse) 1 A, Wert: offen | Fuse_1206 |
| C1, C2 | 220 µF Elko und 100 nF Keramik, 5 V gegen GND | CP_Radial_D6.3mm, C_0805 |

## Layout (Entwurf, offen)

`infoterm-v1.kicad_pcb`: 71,12 x 63,5 mm, 2 Lagen, Leiterbahn 0,3 mm (Strom 0,5 mm), Via 0,8 / 0,4 mm, GND Fläche auf beiden Lagen, 4 Löcher M3 (3,81 mm vom Rand), 7 Testpunkte rechts (TP1 SCK, TP2 MOSI, TP3 3V3, TP4 5V, TP5 VBUS_RAW, TP6 und TP7 GND), Silkscreen mit Titel, D1 mini Umriss (ESP8266, USB rechts), Beschriftung J3 und Rückseite (Copyright, Version, Repo). D1 mini gedreht: Antenne links, ragt rund 4 mm über die Platinenkante, USB Ende rechts. Links an der Kante kein Kupfer (Annahme, offen: Lage mit echtem D1 mini auf einem 1:1 Ausdruck prüfen). Footprints aus der KiCad Bibliothek, Routing mit eigenem Skript. Eigene Abstands und Verbindungsprüfung ohne Befund, das ersetzt keinen DRC (DRC in KiCad offen).

Annahmen (offen):
- Display Modul steckt in J3 und ragt über den oberen Rand hinaus, D1 mini liegt mit dem USB Ende links. Pin 1 von J3 ist VCC: Seite am Modul prüfen.
- J5 (HRO TYPE-C-31-M-12) steht bündig an der unteren Kante, Schnittstelle zum Gehäuse (Schlitz) offen.
- Vor dem DRC alle Zonen füllen (Taste B).

## Offen

- Layout und Gerber fehlen.
- Strom von Summer und Hintergrundlicht (D0 direkt) nicht gemessen.
- Stromversorgung (Vorschlag): USB-C Buchse J5 auf dem PCB speist den 5V Pin des D1 mini. Netzteil: 5 V, Strom nicht gemessen.
- D1 mini nie gleichzeitig über seine eigene USB Buchse und J5 speisen, bevor geklärt ist, ob der Klon eine Diode am 5V Pin hat (offen).
- Symbol J5 nutzt jetzt die Original Pad Namen (A1, A4, A5, A9, A12, B1, B4, B5, B9, B12, SH).
- Buchse hat 0,5 mm Raster: Handlöten ohne Mikroskop heikel, Fertiger Bestückung oder grössere Buchse prüfen (offen).
- Gehäuse: PCB am Rand verschrauben, Schlitz für die Buchse einplanen (offen).
