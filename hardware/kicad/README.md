<!-- SPDX-License-Identifier: CC-BY-4.0 -->
<!-- Copyright (C) 2026 FerienBSD aka Marcel Sauder -->

# InfoTerm v1: Schaltplan (KiCad)

Stand 23.11.2026. Vorschlag, offen: `infoterm-v1.kicad_sch` wurde als Text geschrieben und nie in KiCad geöffnet oder geprüft (kein ERC). Beim Öffnen können Fehler auftreten. Schaltung nach `hardware/Spezifikation.md`.

Hardware: CERN-OHL-P-2.0.

## Bauteile (Vorschlag, Muster ab 5 Stück)

| Ref | Teil | Footprint (Vorschlag) |
|---|---|---|
| J1, J2 | Buchsenleiste 1x8, 2,54 mm, für D1 mini (Reihenabstand 22,86 mm: offen, am Board messen) | PinSocket_1x08 |
| J3 | Buchsenleiste 1x14 für Display MSP2807 | PinSocket_1x14 |
| Q1 | BC557 (PNP), Beinchen C, B, E im Datenblatt prüfen | TO-92 |
| R1 | 1,5 kΩ | Axial DIN0207 |
| J4 | Pfostenstecker 1x2 für Summer | PinHeader_1x02 |

## Offen

- Layout und Gerber fehlen.
- Strom von Summer und Hintergrundlicht (D0 direkt) nicht gemessen.
- Stromversorgung nur über USB am D1 mini.
