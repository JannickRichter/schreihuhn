# SCHREIHUHN

Ein sprachgesteuertes Pixel-Art-Jump'n'Run als native Firmware für den **M5Stack StopWatch** (C152, ESP32-S3, rundes 466×466-AMOLED, ES8311 Mikrofon & Lautsprecher, CST820B Touch, 2 Tasten).

**Reden** lässt das Huhn laufen, **Schreien** lässt es springen und in der Luft flattern. Eine prozedurale, garantierte lösbare Endlos-Welt aus Inseln, brüchigen Brücken, Hindernissen und Coins belohnt jeden Run mit Coins für den Hühner-Shop (HENNI, GOLDIE, NINJA) und einem persistenten Highscore.

## Architektur

- `lib/schreihuhn_core/`: Hardwareunabhängige C++17-Spiellogik, Mikrofon-Klassifizierung (DC-Filter, dBFS-RMS, adaptive Kalibrierung, EMA-Glättung, Hysterese, Sound-Gating), Physik & brüchige Brücken, prozeduraler Map-Generator inkl. Lösbarkeitsprüfung, Software-Renderer (`155×155` PICO-8-Canvas), Screen-Flow & Shop-Persistenz.
- `src/platform/`: M5Unified/M5GFX/ESP32-S3-Adapter für Display (`465×465` zentriert mit 3×-Pixel-Upscaling), ES8311-Codec-Multiplexing (Mikrofon/Lautsprecher), Touch/Buttons, NVS-Speicherung (`Preferences`) und Haptik.
- `test/`: Host-Unit-Tests (`AT-02`–`AT-21`) mit Unity unter `[env:native]`.
- `spec/` & `docs/`: Fachliche Spezifikation und Software-Architektur.

## Voraussetzungen

- [PlatformIO Core](https://platformio.org/) (`pio`, getestet mit 6.2.0)
- Alle Abhängigkeiten sind in `platformio.ini` gepinnt (`espressif32 @ 6.12.0`, `M5Unified @ =0.3.0`, `M5GFX @ =0.3.0`, `M5PM1 #1.0.7`, `M5IOE1 #1.0.9`, `Unity @ =2.6.1`).

## Build-, Flash- und Test-Befehle

| Zweck | Befehl |
|-------|--------|
| Firmware bauen | `pio run -e stopwatch` |
| Debug-Firmware bauen (`DEBUG_LOG` + `SIM_INPUT`) | `pio run -e stopwatch_debug` |
| Save-Reset-Firmware bauen (`RESET_SAVE`) | `pio run -e stopwatch_reset` |
| Firmware flashen | `pio run -e stopwatch -t upload` |
| Serieller Monitor | `pio device monitor -b 115200` |
| Host-Unit-Tests (`AT-02`–`AT-21`) | `pio test -e native` |

## Build-Flags

Die Debug- und Service-Flags können über die vordefinierten Environments oder eigene `-D`-Flags aktiviert werden:

- `-DDEBUG_LOG`: Gibt einmal pro Sekunde Telemetrie über Serial (`115200 Baud`) aus:
  `[DBG] lvl=<dB> floor=<dB> talk=<dB> shout=<dB> state=<STILL|TALK|SHOUT> fps=<n> x=<px> heap=<bytes>`
- `-DSIM_INPUT`: Aktiviert die Tastensimulation während eines Runs (linke Taste halten = `TALK`, rechte Taste halten = `SHOUT`, beide gleichzeitig nicht als Pause gewertet — kurzes Drücken beider Tasten gemeinsam oder Touch oben rechts pausiert). Gedacht für Funktionsprüfungen ohne Mikrofon oder in lauter Umgebung.
- `-DRESET_SAVE`: Löscht den NVS-Namespace `schreihuhn` einmalig beim Booten vor dem Laden (`AppController::init`), sodass ein definierter Ausgangszustand (0 Coins, 0 m Highscore, nur HENNI frei) vorliegt. **Nach einmaligem Einsatz wieder ohne `-DRESET_SAVE` flashen!**

## Tuning

Alle Gameplay-, Mikrofon-, Display-, Timing- und Shop-Konstanten (`K_*`) liegen zentral in [`lib/schreihuhn_core/include/schreihuhn/config/tuning.h`](lib/schreihuhn_core/include/schreihuhn/config/tuning.h). Hinweise zur Kalibrierung auf der Ziel-Hardware stehen in [`TESTING.md`](TESTING.md).
