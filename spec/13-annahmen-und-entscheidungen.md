# 13 – Annahmen & Entscheidungen

Dieses Dokument hält fest, welche Punkte aus der ursprünglichen Idee **bewusst interpretiert**
wurden. Die implementierende KI soll diese Entscheidungen **nicht** erneut infrage stellen, sondern
umsetzen. Eigene Entscheidungen während der Implementierung werden unten ergänzt.

## 13.1 Spezifikations-Entscheidungen

| # | Thema | Entscheidung | Begründung |
|---|-------|--------------|------------|
| E-01 | Scroll-Verhalten | Kein Auto-Scroll; Huhn bewegt sich nur bei Stimme. | Entspricht der Idee "Reden = laufen"; Stille = stehen. Zeitdruck kommt von den Brücken. |
| E-02 | Map | Endlos & prozedural statt fester Level. | Score "wird immer höher, je weiter man geht" → endlos passt; kein Level-Design-Aufwand. |
| E-03 | Hindernis-Kontakt | Hindernisse stoppen, töten aber nicht. | Idee: "Wenn man nicht trifft, fällt man ins Void" – einzige Todesursache ist der Fall. |
| E-04 | Brücken-Timer | Standzeit pro Segment (0,7 s), nicht regenerierend. | "Brechen ein, wenn man zu lange drauf stehen bleibt"; zügiges Laufen ist sicher. |
| E-05 | Schreien am Boden | Immer Sprung; gehaltener Schrei = erneuter Sprung nach Landung + Flattern in der Luft. | Einfach erklärbar: "Schreien = springen & flattern". |
| E-06 | Lautstärke-Schwellen | Automatische Kalibrierung pro Run statt Einstellungsmenü. | Hauptmenü soll laut Idee nur Coins, Hühner, Start, Highscore enthalten. |
| E-07 | Hühner | Rein kosmetisch, Preise 0 / 50 / 150. | MVP-Balance einfach halten; ca. 2 bzw. 5 Runs zum Freischalten. |
| E-08 | Auswahl vs. Kauf | Kein separater "Wählen"-Button: Start mit angezeigtem Huhn = Auswahl; Kauf wählt automatisch. | Weniger Bedienschritte auf kleinem Display. |
| E-09 | Ton An/Aus | Kleines Symbol im Hauptmenü (SOLL). | Wunsch, Sound-Rückkopplung später auszuprobieren → einfacher Schalter zum Vergleichen. |
| E-10 | Sounds im Spiel | Nur Coin-Sound (≤ 150 ms) während der Mikrofon-Auswertung; kein Sprung-Sound. | Minimiert Rückkopplung; Gating hält den Zustand. |
| E-11 | Abbruch über Pause | Coins werden gutgeschrieben, Score zählt für Highscore. | Fair und einfach; kein Frust. |
| E-12 | Tastenbelegung | Links = zurück/abbrechen/Nein, rechts = vor/bestätigen/Ja; Langdruck rechts = Aktion, Langdruck links = Ton. | Konsistentes Links/Rechts-Prinzip, alle Funktionen auch ohne Touch erreichbar. |
| E-13 | Auflösung | 155 × 155 logisch, Faktor 3. | Echter Pixel-Look, gute Performance, ausreichend Sichtweite (~100 px voraus). |
| E-14 | Palette | PICO-8 (16 Farben). | Bekannt, stimmig, frei nutzbar; einfache Paletten-Swaps für Hühner. |
| E-15 | Name | Arbeitstitel "SCHREIHUHN". | Kann später ohne Spec-Änderung ersetzt werden (nur UI-Text). |

## 13.2 Offene Punkte für spätere Versionen (nicht MVP)

- Empfindlichkeits-Einstellung bzw. Lautstärke-Regler für Sounds.
- Weitere Hühner mit unterschiedlichen Fähigkeiten (z. B. längere Flatter-Ausdauer).
- Weitere Hindernisse (bewegliche Plattformen, Gegner), Themes/Welten, Musik.
- Mehrere Highscores, Statistik (gesammelte Coins gesamt, Anzahl Runs).
- Nutzung der IMU (z. B. Schütteln für Spezialaktion) oder der Vibration als Rückmeldung bei Brücken.

## 13.3 Tuning-Hinweise

| Symptom auf Hardware | Zuerst anpassen |
|----------------------|-----------------|
| Huhn läuft bei Stille | `K_TALK_OFFSET_DB` erhöhen |
| Flüstern reicht nicht zum Laufen | `K_TALK_OFFSET_DB` senken, `K_MIC_GAIN` erhöhen |
| Normales Reden löst Sprung aus | `K_SHOUT_OFFSET_DB` / `K_SHOUT_ABS_MIN_DB` erhöhen |
| Schreien springt nicht zuverlässig | `K_SHOUT_OFFSET_DB` senken, `K_SHOUT_MIN_FRAMES` = 1 |
| Coin-Sound triggert Steuerung | `K_VOLUME` senken, `K_GATE_TAIL_MS` erhöhen |
| Zu schwer / zu leicht | Chunk-Gewichte, Stufen-Grenzen, `K_BRIDGE_STABILITY` |

## 13.4 Implementierungsentscheidungen

> Von der implementierenden KI zu ergänzen: Datum, Entscheidung, Begründung, betroffene IDs.

| Datum | Entscheidung | Begründung | Betrifft |
|-------|--------------|------------|----------|
| 2026-11-13 | Abhängigkeiten in `platformio.ini` gepinnt (`espressif32 @ 6.12.0`, `M5Unified @ =0.3.0`, `M5GFX @ =0.3.0`, `M5PM1 #1.0.7`, `M5IOE1 #1.0.9`, `Unity @ =2.6.1`) | Reproduzierbare Builds mit nativer `board_M5StopWatch`-Unterstützung inkl. CO5300-AMOLED, CST820B-Touch, ES8311-Audio und PM1/IOE1. | NFR-07, DoD |
| 2026-11-13 | ES8311-Codec-Multiplexing im `M5AudioAdapter` ohne Power-Cut (`Mic.end()` <-> `Speaker.begin()`) | Da I2S/ES8311 auf dem StopWatch MCLK/BCLK/WS zwischen Mic (`I2S_NUM_1`) und Speaker (`I2S_NUM_0`) teilt, schaltet der Adapter für kurze UI-/Coin-Sounds sauber zwischen Mic- und Speaker-Modus um und blockiert während + `K_GATE_TAIL_MS` nach dem Sound das Mikrofon (Sound-Gating). | FR-AU-10…13, FR-AU-20…23 |
| 2026-11-13 | `M5DisplayAdapter` streamt `155×155` als 3-Zeilen-SRAM-Streifen (`465×3`, `swap565_t`) zentriert (`(1,1)` auf `468×468`) direkt in das Display-Framebuffer-Panel und kapselt `display()` mit `bus->beginTransaction()`, `bus->wait()`, `gpio_hi(cs)` und `bus->endTransaction()` | In M5GFX 0.3.0 / Arduino-Build überschreibt `Bus_SPI::endTransaction()` `SPI_USER_REG` (`SPI_DOUTDIN`/`SPI_USR_MISO`), während `Panel_AMOLED_Framebuffer` leere `beginTransaction()`/`endTransaction()` erbt und kein CS-High ausführt; explizites Bus-Transaction-Wrapping stellt QSPI-Befehle wieder her, das 3-Zeilen-Streaming spart 432 KB PSRAM und hebt die Framerate auf ~32 FPS. | FR-UI-01, NFR-05, NFR-06 |
| 2026-11-13 | Statische Allokation von `Canvas155` (24 KB), `AppController` und Audio-Doppelpuffern in `src/main.cpp` | Die Standard-Stackgröße des Arduino-`loopTask` auf ESP32-S3 beträgt 8 KB; statische Allokation im BSS verhindert Stack-Overflow und Dynamic Allocations im Frame-Loop. | NFR-06 |
| 2026-11-13 | `K_DYING_MAX_SEC = 0.8f` für Fall-Animation bis `Game Over` | Erfüllt den geforderten Zielbereich „0,5–1,0 s nach Auslöser" (`spec/05` 5.3 / `spec/06` 6.2), falls das Huhn oberhalb von `K_VOID_Y` schnell herausfällt. | UC-09, FR-GP-30 |
| 2026-11-13 | `SolvabilityChecker::maxGapPx` rundet `0.80 * W_max` abwärts auf 8-px-Vielfache (ergibt `72 px` Start-Limit für `GAP_L`) | Liegt im spezifizierten Zielkorridor `GAP_L = 48..80 px` (auf Startniveau) und garantiert bei aktiviertem `Flap` sichere Überquerbarkeit über alle Stages. | FR-MP-36, AT-14 |
| 2026-11-13 | Erweiterung des `5×7`-Bitmap-Fonts um UTF-8-dekodierte Umlaute (`Ä`, `Ö`, `Ü`) | Ermöglicht spec-konforme deutsche UI-Texte wie `GENÜGEND` / `UNGÜLTIG` / `ZURÜCK` ohne grafische Platzhalter. | FR-UI-03, spec/06 6.5 |
| 2026-11-13 | `M5StorageAdapter` verwendet ESP32-`Preferences`-Namespace `"schreihuhn"` mit `load/save/reset` sowie `-DRESET_SAVE`-Build-Environment (`stopwatch_reset`) | Ermöglicht sauberes Zurücksetzen für Tests ohne Code-Edit; Save-Validierung liegt vollständig im hardwareunabhängigen `SaveData::sanitize()`. | FR-PS-01…05, FR-SH-06 |
| 2026-11-13 | Kaltstart-AMOLED-Fix: `-DBOARD_ID=30`, `50 ms` Pre-Begin-Delay, expliziter `M5IOE1`-Reset (`gpio8` L3B-Rail, `gpio4`/`gpio5` Reset mit `60 ms` Nachlauf) + Touch-Reinit + vollständige CO5300-QSPI-Init-Sequenz in `M5DisplayAdapter::init()` | `M5GFX`' `stopwatch_power_on()` wartet nach `IOE1 0x18`-Reset nur `2 ms`, bevor `Panel_StopWatch` `0x11` (`Sleep Out`) sendet; nach Kaltstart aus 0 V verpasst der CO5300 die Initialisierung und `Panel_AMOLED_Framebuffer::init()/setSleep()` re-initialisiert das zugrundeliegende Panel nicht erneut. Durch `-DBOARD_ID=30` und expliziten Kaltstart-Reset + QSPI-Reinit startet der Screen nach jedem Ausschalten zuverlässig auf Akku und USB. | FR-UI-01, NFR-07 |
| 2026-11-13 | UI/HW-Feinschliff nach Gerätetest (HW-01/02/05/07): Minimaler schwarzer Splash (`1000 ms`) mit zentriertem Huhn-Icon; großzügige Safe-Circle-Touch-Hitboxen inkl. Display-Offset-Korrektur in `M5InputAdapter` sowie Chord-Unterdrückung bei gleichzeitigem Drücken beider Hardware-Tasten; Brücken-Wackelanimation vertikal (+1 px) ohne horizontales Überlappen und statisches Durchhängen nach Verlassen; `M5AudioAdapter` ohne doppeltes Gating (`triggerSoundGate(0)`) mit Verwerfen der ersten 3 ES8311-ADC-Start-Frames (60 ms) | Behebt Feedback aus erstem StopWatch-Test: zu kleine/mittige Touch-Ziele an den runden Pfeilen und unten am Lautsprecher-Icon, ungewollte Doppelauslösung bei gleichzeitigem Tastendruck, horizontales Segment-Ineinanderbuggen bei Brücken sowie unnötig langes Audio-Gating + ADC-Umschaltknackser. | FR-UI-01, FR-UI-P3, FR-AU-10…11, FR-GR-13 |
| 2026-11-13 | Erweiterte Pause/GameOver-Touch-Hitboxen + Feature-Spec `14-welten-und-hindernisse.md`: 3 neue Hindernis-Klassen (`Overhang`, `WideBarrier`, `Pillar`) inkl. 4 Technikwechsel-Chunks (`ObstUnder`, `ObstWide`, `ObstStair`, `ComboUO`) sowie 4 dynamische Biome (`Greenland`, `Taiga`, `Snow`, `Lava`) mit Bayer-Dither-Überblendung und fest allokiertem Partikel-/Deko-Pool; Unterwasserwelt im Backlog erfasst | Ermöglicht abwechslungsreiches Gameplay (Unterlaufen vs. Springen vs. Flattern) und klare Biome-Atmosphäre auf dem 155×155-PICO-8-Display ohne Heap-Allokationen oder Sichtbehinderung; „Tiger" aus Spracheingabe eindeutig als Nadelwald-Biom „Taiga" interpretiert. | FR-OB-01…12, FR-BM-01…31, FR-UI-P3 |
| 2026-11-14 | Erweiterung `spec/14`: kantige Eisberg-Silhouetten im Schnee-Biom, modulare `IChunkDecorator`-Kette (statisch ohne Heap) im `MapGenerator`, 2 neue reguläre Biome (`Jungle` mit Tempel/Dunst/Sonnenstrahlen/leichtem Regen, `Sand` mit Sonne/Dünen/Kakteen) sowie Weißes-Päckchen-Perk mit mühelosem `TALK`-Flugmodus, Deckenbarriere (`K_PERK_CEILING_Y`) und temporär blinkendem `Candy`-Biom | Erfüllt erweiterte Biome-Vielfalt und klare Parallax-Formen bei strengem Verzicht auf dynamische Frame-Loop-Allokationen; obere Deckenbarriere hält das fliegende Huhn stets sicher unterhalb des HUD im sichtbaren runden Display. | FR-BM-10…22, FR-DC-01…03, FR-PK-01…06 |
