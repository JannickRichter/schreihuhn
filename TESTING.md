# TESTING.md – Hardware-Abnahme & Tuning-Leitfaden

Dieses Dokument ergänzt die automatisierten Host-Unit-Tests (`pio test -e native`, Abdeckung `AT-02`–`AT-21`) und den Firmware-Build (`AT-01`) um die manuelle Hardware-Abnahme (**Teil B**, `HW-01`–`HW-10` gemäß [`spec/11-abnahme-und-dod.md`](spec/11-abnahme-und-dod.md)) sowie das Feintuning der `K_*`-Konstanten.

## Vorbereitung für Hardware-Tests

1. **Sauberer Ausgangszustand (optional):**
   ```bash
   pio run -e stopwatch_reset -t upload
   ```
   Danach sofort Produktiv- oder Debug-Firmware flashen, damit der Speicher nicht bei jedem Boot zurückgesetzt wird.
2. **Debug-Build mit Telemetrie & Tasten-Fallback flashen:**
   ```bash
   pio run -e stopwatch_debug -t upload
   pio device monitor -b 115200
   ```
   Erwartete serielle Ausgabe im 1-Sekunden-Takt:
   ```text
   [DBG] lvl=-58.4dB floor=-61.0dB talk=-49.0dB shout=-30.0dB state=STILL fps=42 x=24 heap=245120
   ```
3. **Produktiv-Build flashen (ohne `SIM_INPUT` / `DEBUG_LOG`):**
   ```bash
   pio run -e stopwatch -t upload
   ```

---

## Hardware-Checkliste (HW-01 … HW-10)

| ID | Prüfung | Durchführung | Erwartung | Status |
|----|---------|--------------|-----------|--------|
| **HW-01** | **Einschalten & Eingabe** | Gerät einschalten, Splash beobachten, linke/rechte Taste und Touch testen. | Pixel-Art-Splash erscheint, nach ≤ 3 s Hauptmenü; linke/rechte Taste steuern Karussell bzw. Zurück/Bestätigen seitenrichtig; Touch-Taps reagieren zuverlässig. *(Falls links/rechts vertauscht sind: `K_BTN_LEFT_IS_BTNA` in `tuning.h` invertieren.)* | [ ] |
| **HW-02** | **Hauptmenü & Safe-Circle** | Hauptmenü sowie Kauf-Dialog auf dem runden AMOLED prüfen. | Coins, Highscore, Hühner-Karussell, Kauf-Preis/Auswahl-Markierung, Ton-Indikator (`L:TON`) und Start-Button sind vollständig im runden Sichtfeld lesbar (nichts abgeschnitten). | [ ] |
| **HW-03** | **Kalibrierung ruhig** | In ruhiger Umgebung auf `SPIELEN` tippen/drücken; Countdown/Kalibrierung (`1,5 s` Ruhe) abwarten; anschließend flüstern (~20 cm), normal sprechen und laut rufen/schreien. | Stille → Huhn steht (`IDLE`); Flüstern → langsames Laufen; normales Reden → schnelleres Laufen; Schreien → Sprung. | [ ] |
| **HW-04** | **Kalibrierung laut** | Hintergrundgeräusch (z. B. Musik/Gespräch) aktivieren, Run starten und während Kalibrierung nicht ins Mikrofon sprechen. | Nach Countdown bleibt Huhn bei Hintergrundgeräusch stehen (`floor_db` passt sich an); gezieltes Schreien löst weiterhin Sprung aus. | [ ] |
| **HW-05** | **Rückkopplung (Ton AN)** | Im Hauptmenü Ton aktiv lassen (`🔊`), Run starten und mehrere Coins einsammeln, während das Gerät normal in der Hand gehalten wird. | Der abgespielte Coin-Ton (`≤ 150 ms`) löst durch Sound-Gating (`K_GATE_TAIL_MS`) danach weder ungewolltes Laufen noch Springen aus. | [ ] |
| **HW-06** | **Flattern** | Während eines Sprungs einen Schrei halten. | Sichtbare Flatter-Animation + Feder-Partikel, flacheres Absinken bis `K_FLAP_STAMINA` (`0,8 s`) aufgebraucht ist; breite Lücken (`GAP_M`/`GAP_L`) werden sicher überwunden. | [ ] |
| **HW-07** | **Brüchige Brücken** | Brücke zügig im Laufen überqueren; in einem zweiten Run auf einer Brücke komplett verstummen und stehenbleiben. | Zügiges Überqueren gelingt ohne Einsturz; Stehenbleiben löst nach `0,35 s` Wackeln und nach `0,7 s` den Einsturz der betretenen Segmente mit Fall in die Lücke → Game Over aus. | [ ] |
| **HW-08** | **Persistenz** | Coins sammeln, neues Huhn im Shop kaufen, Ton-Einstellung umschalten, dann Gerät ausschalten/resetten und wieder einschalten. | Gesammelte Coins, Highscore, freigeschaltetes & gewähltes Huhn sowie Ton-Status sind nach dem Neustart identisch vorhanden. | [ ] |
| **HW-09** | **Performance & Langzeit** | Mit `stopwatch_debug` spielen und mindestens 30 Minuten am Stück (oder mehrere Runs + Idle) seriell loggen. | `fps >= 30`, `heap` bleibt über den gesamten Zeitraum stabil (kein kontinuierlicher Abfall / Speicherleck), kein Watchdog-Reset. | [ ] |
| **HW-10** | **Spielspaß & Kurve** | Mehrere Runs als unvorbereitete Testperson spielen. | Stufe 1 (`0..99 m`) erlaubt im 1. Versuch realistisch ≥ 50 m; Stufe 2 (`≥ 100 m`) und Stufe 3 (`≥ 300 m`) steigern die Herausforderung spürbar, ohne unfaire Stellen zu erzeugen. | [ ] |

---

## Tuning-Leitfaden (`tuning.h`)

Alle Konstanten befinden sich in [`lib/schreihuhn_core/include/schreihuhn/config/tuning.h`](lib/schreihuhn_core/include/schreihuhn/config/tuning.h).

### Häufige Symptome & passende Parameter

| Beobachtung auf Hardware | Relevante Konstanten in `tuning.h` | Richtung / Maßnahme |
|--------------------------|------------------------------------|---------------------|
| **Huhn läuft bei Stille** | `K_TALK_OFFSET_DB`, `K_FLOOR_MAX_DB` | `K_TALK_OFFSET_DB` von `12.0f` schrittweise erhöhen (z. B. `14.0f`–`16.0f`). Prüfen per `DEBUG_LOG`, ob `lvl` knapp über `talk` liegt. |
| **Flüstern / leises Reden reicht nicht zum Laufen** | `K_TALK_OFFSET_DB`, `K_MIC_GAIN` | `K_TALK_OFFSET_DB` leicht senken (z. B. `9.0f`–`10.0f`) oder `K_MIC_GAIN` erhöhen (`1` → `2`..`3`). |
| **Normales Reden löst bereits Sprung (`SHOUT`) aus** | `K_SHOUT_OFFSET_DB`, `K_SHOUT_ABS_MIN_DB` | `K_SHOUT_OFFSET_DB` (`28.0f` → `32.0f`) und/oder `K_SHOUT_ABS_MIN_DB` (`-30.0f` → `-25.0f`) anheben. |
| **Schreien springt verzögert oder unzuverlässig** | `K_SHOUT_OFFSET_DB`, `K_SHOUT_ABS_MIN_DB`, `K_SHOUT_MIN_FRAMES` | `K_SHOUT_OFFSET_DB` / `K_SHOUT_ABS_MIN_DB` senken oder `K_SHOUT_MIN_FRAMES` von `2` auf `1` reduzieren. |
| **Coin-Sound triggert Steuerung (Rückkopplung)** | `K_VOLUME`, `K_GATE_TAIL_MS` | Lautstärke `K_VOLUME` reduzieren (`40` → `25`–`30`) und/oder `K_GATE_TAIL_MS` verlängern (`150` → `220`). |
| **Linke & rechte Taste vertauscht** | `K_BTN_LEFT_IS_BTNA` | Zwischen `true` und `false` umschalten (`BtnA` vs. `BtnB`). |
| **Display-Helligkeit im Freien zu dunkel / zu hell** | `K_BRIGHTNESS` | Prozentwert (`0..100`, Standard `60`) anpassen. |
| **Spielgefühl zu schwer / zu leicht** | `K_JUMP_VY`, `K_GRAVITY`, `K_FLAP_STAMINA`, `K_BRIDGE_STABILITY`, `K_CHUNK_WEIGHTS_S1..S3` | Nach `K_PHYS_*`-Änderungen immer `pio test -e native` laufen lassen, um `AT-06`–`AT-15` (inkl. Lösbarkeits-Fuzztest) zu prüfen! |
