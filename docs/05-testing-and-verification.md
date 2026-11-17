# 05 – Testen & Verifikation

Alle automatisierten Abnahmetests (`AT-01` bis `AT-21` aus `spec/11`) müssen vor Abschluss der
Implementierung ohne Hardware auf dem Host grün sein (`pio test -e native` und `pio run -e stopwatch`).

---

## 5.1 Zuordnung der Abnahmetests (`AT-01`–`AT-21`)

| Test-ID | Ort | Geprüfte Klasse / Methode | Konkrete Assertions |
|---------|-----|---------------------------|---------------------|
| **AT-01** | CLI (`pio run`) | Gesamte Firmware | `pio run -e stopwatch` baut ohne Fehler und ohne Warnungen in `src/` und `lib/schreihuhn_core/` (`-Wall -Wextra`) |
| **AT-02** | `test_mic_classifier` | `MicClassifier::processFrameDb()` | Synthetische dB-Folgen für `STILL`, `TALK`, `SHOUT`; Hysterese (3 dB) beim Abfall; `K_SHOUT_MIN_FRAMES` (1 Frame über `shout_db` reicht nicht, 2 Frames schalten auf `SHOUT`); `intensity` in `[0, 1]` |
| **AT-03** | `test_mic_classifier` | `MicClassifier::finishCalibration()` | Median-Berechnung; Clamping für Floor `-80`, `-60`, `-45`, `-20` dBFS auf `[-70, -40]`; Invariante `shout_db >= talk_db + 10 dB` & `>= K_SHOUT_ABS_MIN_DB` |
| **AT-04** | `test_mic_classifier` + `Physics` | `MicClassifier` + `RunSession` | Ab Schrei-Beginn: Sprung innerhalb ≤ 100 ms simulierter Zeit; ab Stille: `vx == 0` innerhalb ≤ 150 ms |
| **AT-05** | `test_mic_classifier` | `MicClassifier::triggerSoundGate()` | Während aktivem Gating: Pegel & Zustand (`TALK`/`STILL`) bleiben eingefroren; Übergang zu `SHOUT` ist während Gating blockiert |
| **AT-06** | `test_physics_and_bridge` | `Physics` / `RunSession` | Mit Startwerten (`K_PHYS_DT = 1/60 s`): Sprunghöhe `33 ± 2 px`, Sprungweite ohne Flattern `47 ± 3 px`, Weite mit Dauer-Schrei `100 ± 8 px` |
| **AT-07** | `test_physics_and_bridge` | `RunSession::tick()` | Identische 5-s-Input-Sequenz einmal mit `frame_dt = 1/30 s` und einmal mit `frame_dt = 1/60 s` → `|x_30 - x_60| <= 1.0 px` |
| **AT-08** | `test_physics_and_bridge` | `Physics` | `stamina` sinkt nur bei `is_flapping && vy >= 0`; beim Steigen unverändert; bei Landung Reset auf `K_FLAP_STAMINA` |
| **AT-09** | `test_physics_and_bridge` | `Physics` | Seitliche Wandkollision setzt `vx = 0` ohne Tod; Stehen auf Hindernis funktioniert; Kopfkollision setzt `vy = 0`; Fall bei `K_MAX_FALL` tunnelt nicht durch `4 px` Brückensegment |
| **AT-10** | `test_physics_and_bridge` | `Physics` | Sprung `70 ms` nach Kantenverlust erfolgreich (`vy < 0`); nach `90 ms` kein Sprung |
| **AT-11** | `test_physics_and_bridge` | `RunSession` | Hitbox-Oberkante `> 155` löst Übergang nach `Dying` → Run-Ende aus |
| **AT-12** | `test_physics_and_bridge` | `RunSession` (Brücken) | Überqueren mit `K_WALK_MIN`: kein Segment stürzt unter Huhn ein; Stehenbleiben: Wackelschwelle ab `0,35 s` Restzeit, Einsturz bei `0,7 s`, keine Regeneration |
| **AT-13** | `test_map_generator` | `MapGenerator` | Gleicher Seed (z. B. `42`) erzeugt bitgenau identische `Island`/`Bridge`/`Obstacle`/`Coin`-Sequenzen über `3000 m` |
| **AT-14** | `test_map_generator` | `MapGenerator` + `SolvabilityChecker` | **Fuzz-Test `seed = 1..1000` je `3000 m` (`24.000 px`):** (a) keine Überlappungen fester Hindernisse/Inseln, (b) alle Lücken `<=` Referenz-Maximalweite (`0.85`/`0.80`), (c) Landezone `>= 24 px` nach Herausforderungs-Chunk, (d) max. 2 gleiche `ChunkType` hintereinander, (e) Stufen-Chunks entsprechen `0..99`, `100..299`, `>= 300 m`, (f) mittlere Coin-Dichte im Zielbereich (`~1 Coin / 8..12 m`), (g) jeder Coin liegt oberhalb der Void-Grenze und in erreichbarer Flug-/Laufbahn |
| **AT-15** | `test_map_generator` | `MapGenerator` | Simulation über `100 km` (`800.000 px`) mit laufendem `pruneBefore()`: Ringpuffergrößen bleiben strikt unter ihren statischen Kapazitäten (`<= 32 / 64 / 32 / 128`) |
| **AT-16** | `test_app_flow_and_shop` | `RunSession` | `score_m == floor((x_max - 24) / 8)`, sinkt bei Stillstand/Kollision nie |
| **AT-17** | `test_app_flow_and_shop` | `RunSession` + `MockStorage` | Coins werden bei Void-Tod **und** Pause-Abbruch gespeichert; Highscore nur bei Überbietung; Clamping auf `K_COINS_MAX` |
| **AT-18** | `test_app_flow_and_shop` | `ShopLogic` | Kauf mit genug Coins zieht ab, schaltet frei und wählt aus; Kauf mit zu wenig Coins lehnt ab; bereits gekauft nicht erneut kaufbar; HENNI immer frei |
| **AT-19** | `test_app_flow_and_shop` | `SaveData::sanitize()` | Fehlende Felder, Werte `> K_COINS_MAX`, `unlocked` ohne Bit 0, `selected` gesperrt, fremde `schema_version` → saubere Defaults/Korrekturen |
| **AT-20** | `test_app_flow_and_shop` | `InputMapper` | Kurz- vs. Langdruck (`< 600 ms` bei Release vs. `>= 600 ms` sofort ohne Release-Event), Tap-Distanz (`< 10 px`, `<= 500 ms`), Lockout `250 ms` / `1000 ms`, gehaltene Tasten nach Screenwechsel unterdrückt |
| **AT-21** | `test_app_flow_and_shop` | `AppController` | Vollständige State-Machine-Übergänge (`Splash → MainMenu → BuyDialog → Countdown → Playing → Paused → Resume → Playing → MainMenu` und `Playing → GameOver → Countdown`) inkl. `MockAudioOut`/`MockHaptics`-Prüfung |

---

## 5.2 Spezielle Test-Rezepte für kritische Tests

### `AT-06` & `AT-07` (Physik-Referenz & Framerate-Unabhängigkeit)

- Für **`AT-06`**: Starte Huhn auf einer breiten Testinsel (`y = 104`, `x = 24`). Gib 1 Tick `SHOUT`
  und danach bis zur Landung entweder `STILL` (ohne Flattern) oder durchgehend `SHOUT` (mit
  Flattern). Messe `min_y` (Sprunghöhe = `104 - (min_y + K_HITBOX_H)`) und `landing_x - 24`.
- Für **`AT-07`**: Erzeuge einen `RunSession`-Run mit festem Seed und spiele eine 5-sekündige
  Misch-Sequenz (`TALK → SHOUT → STILL → SHOUT`) einmal in `30 Hz`-Schritten (`dt = 1/30 s`) und
  einmal in `60 Hz`-Schritten (`dt = 1/60 s`) in den Timestep-Accumulator ein. Prüfe
  `TEST_ASSERT_FLOAT_WITHIN(1.0f, x_60, x_30)`.

### `AT-14` (Fuzz-Test 1000 Seeds × 3000 m)

- Um `AT-14` auf dem Host schnell (< 2 Sekunden Laufzeit) auszuführen, prüft der Test den
  `MapGenerator` direkt im Streaming-Modus: `cursor` schreitet in `160 px`-Schritten bis
  `24.024 px` (`3000 m`) voran; nach jedem Schritt werden neu generierte Elemente auf alle
  Invarianten geprüft und links außerhalb `cursor - 32` geprunt.

---

## 5.3 Empfohlene Implementierungs-Reihenfolge (Meilensteine)

1. **Projekt-Setup & Core-Skelett:** `platformio.ini`, `tuning.h`, `math_types.h`, `ring_buffer.h`,
   HAL-Header; erstes `pio test -e native` und `pio run -e stopwatch` grün.
2. **Mikrofon- & Eingabe-Logik:** `MicClassifier`, `InputMapper`, `SaveData`, `ShopLogic` + Tests
   `AT-02`–`AT-05`, `AT-18`–`AT-20`.
3. **Physik & Map-Generator:** `Physics`, `SolvabilityChecker`, `MapGenerator`, `RunSession` + Tests
   `AT-06`–`AT-17`.
4. **Rendering & App-Controller:** `palette.h`, `sprites.h`, `font5x7.h`, `WorldRenderer`,
   `UIRenderer`, `AppController` + Test `AT-21`.
5. **ESP32-Platform-Adapter:** `M5DisplayAdapter`, `M5InputAdapter`, `M5StorageAdapter`,
   `M5HapticsAdapter`, `M5AudioAdapter`, `main.cpp` inkl. `-DDEBUG_LOG` und `-DSIM_INPUT`.
6. **Vollständige Verifikation (Teil A):** `pio test -e native`, `pio run -e stopwatch` und
   `pio run -e stopwatch_debug` warnungsfrei grün.
7. **Dokumentation & DoD:** `README.md`, `TESTING.md` (Hardware-Checkliste `HW-01`–`HW-10`),
   Abschnitt `13.4` in `spec/13-annahmen-und-entscheidungen.md` ergänzen.
