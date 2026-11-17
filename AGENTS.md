# AGENTS.md

Leitfaden für KI-Agenten, die in diesem Repository arbeiten.

## Projekt

**SCHREIHUHN** – ein sprachgesteuertes Pixel-Art-Jump'n'Run als native Firmware für den
**M5Stack StopWatch (C152, ESP32-S3, rundes 466×466-AMOLED, Mikrofon, Lautsprecher, Touch, 2 Tasten)**.
Reden → Huhn läuft, Schreien → Huhn springt/flattert. Endlose Map mit Inseln, brüchigen Brücken,
Hindernissen und Coins; Hauptmenü mit Coins, Highscore, 3 Hühnern (2 kaufbar) und Start.

## Single Source of Truth

1. **Fachliche Spezifikation:** [`/spec`](spec/README.md) (`spec/README.md`, dann `spec/01` … `spec/13`).
   **Lies sie vollständig, bevor du Code schreibst.**
2. **Software-Architektur:** [`/docs`](docs/README.md) (`docs/README.md`, dann `docs/01` … `docs/05`).
   Definiert Schichtenmodell, Verzeichnisstruktur, `platformio.ini`, C++-Modulschnittstellen,
   HAL-Adapter, Rendering-Pipeline und die Zuordnung der Abnahmetests `AT-01`–`AT-21`.

Bei Widersprüchen innerhalb der Spezifikation gilt: spezifischeres Dokument (05–09) vor allgemeinem
(01–04); Tabellenwerte vor Fließtext; `13-annahmen-und-entscheidungen.md` klärt
Interpretationsfragen. Bei Widersprüchen zwischen `/docs` und `/spec` hat `/spec` Vorrang.

## Arbeitsmodus: autonom, ohne Rückfragen

- Es gibt **keinen Menschen im Loop**. Stelle keine Rückfragen und warte nicht auf Bestätigung.
- Ist etwas unklar oder technisch nicht wie spezifiziert umsetzbar: triff die naheliegendste,
  spec-konforme Entscheidung, setze sie um und dokumentiere sie in
  `spec/13-annahmen-und-entscheidungen.md` → Abschnitt **13.4 Implementierungsentscheidungen**.
- Ändere die übrigen Spec-Dokumente **nicht** inhaltlich. Ausnahme: offensichtliche Tippfehler.
- Halte die Architektur aus [`/docs`](docs/README.md) ein, insbesondere die strikte Trennung von
  `lib/schreihuhn_core/` und `src/platform/`, sowie RB-01 … RB-07 (`spec/02`) und NFR-08/NFR-09
  (hardwareunabhängig testbare Spiellogik, zentrale `K_*`-Konstanten).

## Vorgehen (empfohlen)

1. PlatformIO-Projekt aufsetzen (`stopwatch`-Environment + `native`-Test-Environment), Abhängigkeiten pinnen.
2. Hardware-Bring-up: Display (logischer 155×155-Puffer, Faktor 3), Touch, Tasten (links/rechts verifizieren), Mikrofon-Pegel, Ton, NVS.
3. Hardwareunabhängige Spiellogik + Unit-Tests (Physik, Kollision, Mikrofon-Klassifizierung, Brücken, Generator inkl. Lösbarkeit, Score, Shop, Persistenz-Validierung, Eingabe-Logik).
4. Rendering, Sprites (Code-Arrays, PICO-8-Palette), HUD.
5. Screens & Screen-Flow, Menü, Shop, Persistenz.
6. Audio, Gating, Haptik, Debug-Flags.
7. Abnahme Teil A (`spec/11`) vollständig grün; `README.md` und `TESTING.md` schreiben.

## Befehle (Soll-Zustand, vom Agenten anzulegen)

| Zweck | Befehl |
|-------|--------|
| Firmware bauen | `pio run -e stopwatch` |
| Flashen | `pio run -e stopwatch -t upload` |
| Serieller Monitor | `pio device monitor -b 115200` |
| Host-Unit-Tests | `pio test -e native` |
| Debug-Build | Build-Flags `-DDEBUG_LOG` / `-DSIM_INPUT` / `-DRESET_SAVE` (als eigene Environments oder dokumentierte Flags) |

Vor jedem Abschluss eines Arbeitsschritts: `pio run -e stopwatch` **und** `pio test -e native`
müssen grün sein.

## Konventionen

- Sprache: Code, Bezeichner, Kommentare, Commit-Messages **Englisch**; UI-Texte **Deutsch** (verbindliche Texte: `spec/06`, 6.5).
- C++17 (bzw. was die Toolchain unterstützt), `-Wall` ohne Warnungen im eigenen Code.
- Alle Tuning-Werte als `K_*`-Konstanten (Name wie in der Spec) an **einem** zentralen Ort, mit Einheit im Kommentar.
- Keine magischen Zahlen, keine dynamischen Allokationen im Frame-Loop, wenn vermeidbar.
- Grafiken und Sounds ausschließlich im Code erzeugt (RB-05); keine externen Assets mit unklarer Lizenz.
- Verweise im Code auf Spec-IDs, wo hilfreich (z. B. `// FR-MP-36`).
- Keine Netzwerkfunktionen.
- Kleine, nachvollziehbare Commits (falls Git verwendet wird); keine Pushes zu Remotes ohne Auftrag.

## Fertig, wenn

Die **Definition of Done** in `spec/11-abnahme-und-dod.md` (Abschnitt 11.3) vollständig erfüllt ist.

## Hardware-Referenzen

- https://docs.m5stack.com/en/core/StopWatch
- https://docs.m5stack.com/en/arduino/stopwatch/program
- https://github.com/m5stack/M5StopWatch-UserDemo (ESP-IDF-Referenz für Peripherie)
