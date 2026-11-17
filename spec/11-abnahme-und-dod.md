# 11 – Abnahme & Definition of Done

Die Abnahme ist zweigeteilt:

- **Teil A – Automatisiert (ohne Hardware):** Muss die implementierende KI selbst vollständig
  erfüllen und nachweisen.
- **Teil B – Hardware-Abnahme:** Wird nach Übergabe von einem Menschen auf dem Gerät durchgeführt.
  Die implementierende KI liefert dafür die Checkliste (`TESTING.md`) und Flash-Anleitung mit.

## 11.1 Teil A – Automatisierte Abnahmetests (Host, `native`)

| ID | Test | Erwartung | Bezug |
|----|------|-----------|-------|
| AT-01 | Firmware-Build `pio run -e stopwatch` | Erfolgreich, ohne Warnungen im eigenen Code | RB-03, NFR-10 |
| AT-02 | Mikrofon-Klassifizierung: synthetische Pegelfolgen (Stille, Reden, Schreien, Übergänge, Flattern um Schwelle) | Korrekte Zustände inkl. Hysterese und `K_SHOUT_MIN_FRAMES` | FR-AU-04…06 |
| AT-03 | Kalibrierung: Median, Begrenzung, `shout ≥ talk + 10` | Korrekt für Floor −80, −60, −45, −20 dBFS | FR-AU-03/04 |
| AT-04 | Latenz-Logik: Schrei-Beginn → Sprung | ≤ 100 ms simulierte Zeit; Stille → `vx = 0` ≤ 150 ms | FR-AU-07, FR-GP-03 |
| AT-05 | Gating: Sound während TALK / SHOUT / STILL | Zustand gehalten; kein neues SHOUT während Gating | FR-AU-10 |
| AT-06 | Physik-Referenzwerte | Sprunghöhe 33 ± 2 px, Weite ohne Flattern 47 ± 3 px, mit Dauer-Schrei 100 ± 8 px (bei Startwerten) | 5.3 |
| AT-07 | Framerate-Unabhängigkeit | Gleiche Eingabesequenz bei 30/60 Render-FPS → Endposition ± 1 px | NFR-02 |
| AT-08 | Ausdauer | Verbrauch nur beim Sinken + Flattern; Reset bei Landung | FR-GP-13 |
| AT-09 | Kollision | Seitlich: Stopp ohne Tod; von oben: stehen; von unten: `vy = 0`; kein Durchtunneln bei max. Fallgeschwindigkeit | FR-GP-20…24 |
| AT-10 | Coyote-Time | Sprung 70 ms nach Kantenverlust möglich, 90 ms nicht | FR-GP-10 |
| AT-11 | Tod | Hitbox-Oberkante > 155 → Run-Ende | FR-GP-30 |
| AT-12 | Brücke | Überqueren bei `K_WALK_MIN` → kein Einsturz unter dem Huhn; Stehenbleiben → Einsturz nach 0,7 s; Wackeln ab 0,35 s; keine Regeneration | FR-MP-11…16 |
| AT-13 | Generator-Determinismus | Gleicher Seed → identische Elementliste | FR-MP-30 |
| AT-14 | Generator-Fuzz (1000 Seeds × 3000 m) | Keine Überlappungen; alle Lücken ≤ Grenzwerte (FR-MP-36); Landezonen vorhanden; ≤ 2 gleiche Chunks in Folge; Stufen korrekt; Coin-Dichte im Ziel­bereich; jeder Coin erreichbar | FR-MP-30…37, NFR-15 |
| AT-15 | Generator-Speicher | 100 km Simulation → Anzahl gehaltener Elemente bleibt beschränkt | FR-MP-31, NFR-06 |
| AT-16 | Score | `floor((x_max − x_start)/8)`, sinkt nie | FR-GP-40 |
| AT-17 | Run-Ende | Coins gutgeschrieben (Tod und Abbruch), Highscore nur bei Überbietung, Clamping auf Maximum | UC-09a, FR-PS-06 |
| AT-18 | Shop | Kauf mit genug/zu wenig Coins; bereits gekauft; Auswahl nach Kauf; HENNI immer frei | FR-SH-01…05 |
| AT-19 | Persistenz-Validierung | Fehlende Felder, Werte außerhalb Bereich, `selected` gesperrt, fremde `schema_version` → Defaults/Korrekturen wie spezifiziert | FR-PS-02/03 |
| AT-20 | Eingabe-Logik Tasten/Touch | Kurz-/Langdruck-Unterscheidung, Tap-Erkennung, Eingabesperre nach Screenwechsel (250 ms / 1000 ms) | FR-UI-P4…P7 |
| AT-21 | Screen-Flow | Alle Übergänge aus 6.2 inkl. Abbruch über Pause → Hauptmenü mit gespeichertem Ergebnis | 6.2, UC-08 |

## 11.2 Teil B – Hardware-Abnahme (Mensch, nach Übergabe)

| ID | Prüfung | Erwartung |
|----|---------|-----------|
| HW-01 | Einschalten | Splash, nach ≤ 3 s Hauptmenü; Tasten/Touch funktionieren, links/rechts korrekt |
| HW-02 | Hauptmenü | Coins, Highscore, Karussell, Kauf, Ton-Toggle wie spezifiziert, alles im runden Sichtbereich |
| HW-03 | Kalibrierung ruhig | Im ruhigen Raum: Flüstern in 20 cm → läuft; normales Reden → läuft schneller; Schreien → springt |
| HW-04 | Kalibrierung laut | Mit Hintergrundgeräusch (Musik/Gespräch): Stille ≠ Laufen nach Kalibrierung, Schreien springt weiterhin |
| HW-05 | Rückkopplung | Coin-Sound löst weder Laufen noch Springen aus (bei Ton an, Gerät in der Hand) |
| HW-06 | Flattern | Gehaltener Schrei: sichtbares Flattern, große Lücken überwindbar |
| HW-07 | Brücken | Zügig drüberlaufen klappt; Stehenbleiben → Einsturz → Fall → Game Over |
| HW-08 | Persistenz | Coins/Highscore/Kauf/Ton-Einstellung nach Aus-/Einschalten erhalten |
| HW-09 | Performance | `DEBUG_LOG`: ≥ 30 FPS, Heap stabil über 30 min |
| HW-10 | Spielspaß | Stufe 1 ist für Anfänger:innen schaffbar (≥ 50 m im 1. Versuch realistisch), Stufe 3 fordernd |

Bei HW-03/04/05 sind Anpassungen der `K_*`-Werte ausdrücklich vorgesehen (kein Fehler der
Implementierung, sondern Tuning).

## 11.3 Definition of Done (für die implementierende KI)

- [ ] Alle **MUSS**-Anforderungen (FR-*, NFR-*) und Must-User-Stories umgesetzt.
- [ ] Alle **SOLL**-Anforderungen umgesetzt oder Abweichung im Entscheidungslog begründet.
- [ ] AT-01 bis AT-21 automatisiert vorhanden und grün (`pio run -e stopwatch` und `pio test -e native`).
- [ ] `README.md` im Projekt-Root: Projektbeschreibung, Build-, Flash- und Test-Befehle, Build-Flags (`SIM_INPUT`, `DEBUG_LOG`, `RESET_SAVE`).
- [ ] `TESTING.md` mit der Hardware-Checkliste HW-01 … HW-10 und Tuning-Hinweisen (welche `K_*` wofür).
- [ ] Alle Tuning-Konstanten zentral, dokumentiert (Einheit + Bedeutung als Kommentar).
- [ ] Entscheidungslog in [13-annahmen-und-entscheidungen.md](13-annahmen-und-entscheidungen.md) um eigene Entscheidungen ergänzt (Abschnitt "Implementierungsentscheidungen").
- [ ] Keine TODOs/Platzhalter-Grafiken in MUSS-Funktionen.
- [ ] Abhängigkeiten gepinnt, Build reproduzierbar aus sauberem Checkout.
