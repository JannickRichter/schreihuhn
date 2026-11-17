# SCHREIHUHN – MVP-Spezifikation

> Arbeitstitel: **SCHREIHUHN** (darf im Code/UI verwendet werden, Name ist nicht final).
> Ein sprachgesteuertes Pixel-Art-Jump'n'Run für den **M5Stack StopWatch (C152)**.
> Man redet → das Huhn läuft. Man schreit → das Huhn springt und flattert.

## Zweck dieses Dokuments

Diese Spezifikation beschreibt das **MVP** vollständig genug, dass eine implementierende KI es
**ohne Rückfragen (kein Human-in-the-Loop)** umsetzen kann. Die Software-Architektur ist
ausdrücklich **nicht** Teil der Spezifikation und wird von der implementierenden Partei selbst
gewählt (siehe [01-vision-und-scope.md](01-vision-und-scope.md)).

## Lesereihenfolge

| # | Dokument | Inhalt |
|---|----------|--------|
| 01 | [Vision & Scope](01-vision-und-scope.md) | Spielidee, Ziele, In/Out of Scope, Priorisierung |
| 02 | [Hardware & Rahmenbedingungen](02-hardware-und-rahmenbedingungen.md) | Zielgerät, Toolchain, technische Constraints |
| 03 | [User Stories](03-user-stories.md) | Alle User Stories inkl. Akzeptanzkriterien |
| 04 | [Use Cases](04-use-cases.md) | Use-Case-Diagramm (High Level) + ausformulierte Use Cases |
| 05 | [Gameplay-Regeln](05-gameplay-regeln.md) | Steuerung, Physik, Map-Elemente, Level-Generierung, Score, Coins |
| 06 | [Screens & Bedienung](06-screens-und-bedienung.md) | Bildschirme, Layout (rundes Display), Touch/Buttons, Screen-Flow |
| 07 | [Audio & Mikrofon](07-audio-und-mikrofon.md) | Lautstärke-Erkennung, Kalibrierung, Sounds, Rückkopplungsschutz |
| 08 | [Grafik & Pixel-Art](08-grafik-pixel-art.md) | Auflösung, Palette, Sprites, Animationen |
| 09 | [Persistenz & Shop](09-persistenz-und-shop.md) | Speicherstände, Hühner, Preise |
| 10 | [Nicht-funktionale Anforderungen](10-nicht-funktionale-anforderungen.md) | Performance, Robustheit, Testbarkeit |
| 11 | [Abnahme & Definition of Done](11-abnahme-und-dod.md) | Abnahmekriterien, Testfälle, DoD |
| 12 | [Glossar](12-glossar.md) | Begriffe |
| 13 | [Annahmen & Entscheidungen](13-annahmen-und-entscheidungen.md) | Getroffene Annahmen, Tuning-Hinweise, Entscheidungslog |
| 14 | [Welten & Hindernisse](14-welten-und-hindernisse.md) | Erweiterungsspezifikation: dynamische Biome, Technikwechsel-Hindernisse & Unterwasser-Backlog |

## ID-Konventionen

| Präfix | Bedeutung | Beispiel |
|--------|-----------|----------|
| `US-xx` | User Story | US-07 |
| `UC-xx` | Use Case | UC-06 |
| `FR-<Bereich>-xx` | Funktionale Anforderung | FR-GP-12 |
| `NFR-xx` | Nicht-funktionale Anforderung | NFR-03 |
| `AT-xx` | Abnahmetest | AT-15 |
| `K_*` | Tuning-Konstante (zentral konfigurierbar) | `K_GRAVITY` |

## Verbindlichkeit

- **MUSS** = verpflichtend für das MVP.
- **SOLL** = umsetzen, sofern kein technischer Hinderungsgrund besteht; Abweichung in
  [13-annahmen-und-entscheidungen.md](13-annahmen-und-entscheidungen.md) dokumentieren.
- **KANN** = optional, nur wenn alles MUSS/SOLL erledigt ist.

Alle Zahlenwerte mit `K_`-Präfix sind **Startwerte für das Tuning**. Sie MÜSSEN zentral an einer
Stelle im Code definiert sein, dürfen aber begründet angepasst werden (Dokumentation im
Entscheidungslog).
