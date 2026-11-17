# SCHREIHUHN – Software-Architektur

Dieses Verzeichnis definiert die verbindliche Software-Architektur für die Implementierung von
**SCHREIHUHN** auf dem **M5Stack StopWatch (C152, ESP32-S3)** und dient als direktes Bindeglied
zwischen der fachlichen Spezifikation ([`/spec`](../spec/README.md)) und der C++17-Codebasis.

> Bei Widersprüchen zwischen dieser Architektur und `/spec` hat die fachliche Spezifikation `/spec`
> Vorrang (siehe auch [`AGENTS.md`](../AGENTS.md)).

## Lesereihenfolge

| # | Dokument | Inhalt |
|---|----------|--------|
| – | [README](README.md) | Zweck, Überblick, zentrale Architektur-Entscheidungen |
| 01 | [Systemkontext & Schichten](01-system-context-and-layers.md) | Schichtenmodell, HAL-Grenze, Verzeichnisstruktur, `platformio.ini` |
| 02 | [Core-Domäne & Module](02-core-domain-and-modules.md) | C++17-Schnittstellen, Tuning-Konstanten, Physik, Generator, Shop, State-Machine |
| 03 | [Laufzeit & Hardware-HAL](03-runtime-and-hardware-hal.md) | Frame-Ablauf, Display-Upscaling, ES8311-Multiplexing, Input, Debug-Telemetrie |
| 04 | [Rendering & Assets](04-rendering-and-assets.md) | PICO-8-Palette, Dithering, Sprite-Format, Paletten-Swap, Screen-Layouts, Font |
| 05 | [Testen & Verifikation](05-testing-and-verification.md) | Zuordnung `AT-01`–`AT-21`, Test-Rezepte, Implementierungs-Meilensteine |

## Die 5 zentralen Architektur-Entscheidungen (Architectural Drivers)

1. **Strikte Trennung von Core und Plattform (`NFR-08`, `RB-04`):**
   Die gesamte Spiellogik – Physik, AABB-Kollision, Map-Generierung mit Lösbarkeitsprüfung,
   Mikrofon-Klassifizierung, Shop-/Persistenz-Logik, Input-Mapping, Screen-State-Machine und das
   155 × 155-Paletten-Rendering – liegt als reines, plattformunabhängiges C++17 (ohne Arduino-,
   ESP-IDF- oder M5Unified-Header) unter `lib/schreihuhn_core/`. Dadurch kompiliert und läuft
   derselbe Code unverändert sowohl in `pio test -e native` auf dem Host-PC als auch in
   `pio run -e stopwatch` auf dem ESP32-S3.

2. **Indizierter 155 × 155-Software-Framebuffer (`Canvas155`, `FR-GR-01`–`03`):**
   Der Core rendert ausschließlich in ein `155 × 155`-Byte-Array (Paletten-Index `0..15`,
   ~24 KB RAM). Der Display-Adapter der Zielplattform skaliert diesen Puffer per Nearest-Neighbor
   mit Faktor 3 unter Verwendung einer 16-Einträge-Lookup-Tabelle (`palette_rgb565[16]`) auf
   `465 × 465` physische Pixel in einen PSRAM-Sprite/Zeilenpuffer und überträgt ihn zentriert
   auf das 466 × 466-AMOLED-Display.

3. **Zero-Heap-Allocation im Game-Loop (`FR-MP-31`, `NFR-05`, `NFR-06`):**
   Alle dynamisch wachsenden bzw. rollenden Welt-Container (`Island`, `BridgeSegment`, `Obstacle`,
   `Coin`, `Particle`) verwenden fest dimensionierte, statische Ringpuffer (`StaticRingBuffer<T, N>`).
   Während eines Runs finden keine `new`/`malloc`/`std::vector`-Reallokationen statt; der
   Speicherverbrauch bleibt über beliebig lange Runs konstant.

4. **Fixed-Timestep-Simulation (`K_PHYS_DT = 1/60 s`, `NFR-02`, `AT-07`):**
   Ein Zeit-Akkumulator im `AppController` bzw. der `RunSession` entkoppelt die deterministischen
   Physik-Ticks (`1/60 s`) von der tatsächlichen Render-Framerate (≥ 30 FPS). Dadurch verhält sich
   das Spiel bei 30 FPS und 60 FPS identisch.

5. **Nicht-blockierendes Audio-/Codec-Multiplexing (`FR-AU-10`–`12`):**
   Da Mikrofon und Lautsprecher auf dem M5Stack StopWatch den ES8311-Codec gemeinsam nutzen,
   steuert ein nicht-blockierender `M5AudioAdapter` ohne `delay()` den Wechsel zwischen
   Aufnahme (`M5.Mic`) und Wiedergabe (`M5.Speaker`), spielt synthetische Tonfolgen sequenziert ab
   und aktiviert über `MicClassifier::triggerSoundGate(...)` automatisch das Rückkopplungs-Gating
   (`K_GATE_TAIL_MS = 150 ms`).
