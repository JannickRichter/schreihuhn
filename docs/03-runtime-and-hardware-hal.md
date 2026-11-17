# 03 – Laufzeit & Hardware-HAL

Dieses Dokument beschreibt die konkrete Anbindung an den **M5Stack StopWatch (C152)** unter
`src/` und das zeitliche Zusammenspiel im Haupt-Loop.

---

## 3.1 Haupt-Loop & Frame-Ablauf

`setup()` initialisiert `M5.begin(cfg)` (inkl. PSRAM-Check, Helligkeit `K_BRIGHTNESS`, Display-Rotation
und Ausrichtung), lädt `SaveData` aus dem `M5StorageAdapter` (bei `-DRESET_SAVE` nach vorherigem
`reset()`), initialisiert `M5AudioAdapter` (prüft Mikrofon-Verfügbarkeit für `FR-UI-03`),
`M5DisplayAdapter` und übergibt alle Adapter an `AppController`.

```mermaid
sequenceDiagram
    participant Loop as "src/main.cpp (loop)"
    participant In as "M5InputAdapter"
    participant Aud as "M5AudioAdapter"
    participant App as "AppController (Core)"
    participant Disp as "M5DisplayAdapter"

    Loop->>Loop: "M5.update(), dt_ms berechnen"
    Loop->>In: "pollInput(SIM_INPUT?)"
    In-->>Loop: "PlatformInput"
    Loop->>Aud: "update(now_ms) + readMicSamples()"
    Aud-->>App: "MicClassifier::processSamples(...)"
    Loop->>App: "tick(dt_sec, PlatformInput)"
    App->>Aud: "ggf. playSound(SoundId)"
    App->>App: "Physics Fixed-Step (K_PHYS_DT) + State"
    Loop->>App: "render(Canvas155)"
    Loop->>Disp: "present(Canvas155)"
    Disp->>Disp: "3x Nearest-Neighbor RGB565 in PSRAM + pushSprite"
```

---

## 3.2 Display-Adapter (`M5DisplayAdapter`, `FR-GR-01`–`03`)

- **Physisches Display:** `466 × 466` px AMOLED (CO5300 über QSPI).
- **Initialisierung:**
  - `M5.Display.setBrightness((K_BRIGHTNESS * 255) / 100);`
  - `M5.Display.fillScreen(TFT_BLACK);` (einmalig, damit der 1-px-Rand um `465×465` schwarz bleibt).
  - `LGFX_Sprite frame_sprite(&M5.Display);` mit `frame_sprite.setPsram(true);`,
    `frame_sprite.setColorDepth(16);`, `frame_sprite.createSprite(465, 465);` (~432 KB in 8 MB OPI PSRAM).
- **3× Upscaling (`present(const Canvas155& canvas)`):**
  - Arbeitet zeilenweise mit einem kleinen Stack-Zeilenpuffer `uint16_t row465[465]`:
    1. Für jede logische Zeile `ly = 0..154`:
       - Für `lx = 0..154`: `uint16_t c = palette_rgb565[canvas.pixels[ly * 155 + lx] & 0x0F];`
         schreibe `c` dreimal hintereinander nach `row465[lx * 3 + 0..2]`.
       - Kopiere `row465` dreimal per `memcpy` in die Zeilen `ly * 3 + 0..2` des `frame_sprite`-Puffers
         (oder via `pushImage` / direkten `getBuffer()`-Pointer).
    2. `frame_sprite.pushSprite(0, 0);` (überträgt per QSPI-DMA ohne sichtbares Flimmern/Tearing).

> Hinweis zur Byte-Reihenfolge (`M5GFX`): Falls `frame_sprite.getBuffer()` direkt beschrieben wird,
> ist bei `M5GFX` 16-bit RGB565 ggf. `setSwapBytes(true)` oder Byte-Swapping in `palette_rgb565` zu
> prüfen – einmal beim Hardware-Bring-up verifizieren (`AGENTS.md` Schritt 2).

---

## 3.3 Audio- & Mikrofon-Adapter (`M5AudioAdapter`, `spec/02` & `spec/07`)

### Hardware-Eigenheit ES8311 (`FR-AU-10`…`12`)

Auf dem M5Stack StopWatch bedient der Codec **ES8311** sowohl das MEMS-Mikrofon als auch den
AW8737A-Lautsprecherverstärker. Da gleichzeitiges Vollduplex-Aufnehmen und -Abspielen je nach
M5Unified-/I2S-Treiberstand nicht garantiert ist, setzt `M5AudioAdapter` ein deterministisches
**Codec-Multiplexing** um:

1. **Modi:** `CodecMode { Idle, MicActive, SpeakerActive }`.
2. **Wann ist `M5.Mic` aktiv?** Nur in `Countdown` und `Playing`, sofern gerade kein Sound läuft
   (`FR-AU-09`). In Menüs, `Paused` und `GameOver` bleibt `M5.Mic` gestoppt und `M5.Speaker` aktiv.
3. **Abspielen eines Sounds (`playSound(SoundId id)`):**
   - Wenn `!sound_enabled_`: sofortiger Return (kein Sound, kein Gating, `FR-AU-20`).
   - Tonfolge aus `sound_catalog.h` in internen Ringpuffer/Array laden (`max. 4 Noten`).
   - Gesamtdauer `total_ms` der Tonfolge berechnen.
   - `mic_classifier_.triggerSoundGate(total_ms)` aufrufen (fügt automatisch `K_GATE_TAIL_MS = 150 ms`
     hinzu, `FR-AU-10`).
   - Falls aktuell `MicActive`: `M5.Mic.end(); M5.Speaker.begin();` und Lautstärke auf
     `(K_VOLUME * 255) / 100` setzen.
4. **Nicht-blockierender Noten-Sequenzer (`update(uint32_t now_ms)`):**
   - Prüft ohne `delay()`, ob die aktuelle Note abgelaufen ist, und startet ggf. die nächste via
     `M5.Speaker.tone(note.freq_hz, note.duration_ms)`.
   - Nach Ablauf der letzten Note: Wenn der `AppController` Mikrofon-Betrieb anfordert (`Countdown`
     oder `Playing`): `M5.Speaker.end(); M5.Mic.begin();` aufrufen.
   - Zeitbilanz für `SND-COIN` (`110 ms`) im Spiel: `110 ms` Sound + Codec-Umschaltung (~20–40 ms) +
     `150 ms` Tail = ~280 ms < `400 ms` (`FR-AU-11`).
5. **Mikrofon-Auslesen (`M5.Mic.record`):**
   - Konfiguration beim Start: `sample_rate = K_MIC_RATE (16000)`, `magnification = K_MIC_GAIN`
     (siehe `M5Unified` Mic-Config).
   - Verwendet **2 alternierende Sample-Puffer** (`int16_t buf[2][320]`, je `20 ms`).
   - Sobald `M5.Mic.record(buf[active], 320, K_MIC_RATE)` einen Block gefüllt hat (über
     `!M5.Mic.isRecording()` bzw. asynchronen Puffer-Fortschritt), wird der gefüllte Block an
     `MicClassifier::processSamples()` übergeben und sofort der andere Puffer gestartet (`FR-AU-08`).
   - Scheitert `M5.Mic.begin()` beim Booten: `mic_available_ = false` → Splash zeigt `MIKROFON-FEHLER`,
     Spiel startet keinen Run (`UC-01` 1a, `UC-05` 5a, `NFR-07`).

---

## 3.4 Input-, Storage- & Haptik-Adapter

### Input (`M5InputAdapter`)

- **Tasten (`FR-UI-P2`, `2.3`):**
  ```cpp
  bool btn_a = M5.BtnA.isPressed();
  bool btn_b = M5.BtnB.isPressed();
  out.btn_left_down  = K_BTN_LEFT_IS_BTNA ? btn_a : btn_b;
  out.btn_right_down = K_BTN_LEFT_IS_BTNA ? btn_b : btn_a;
  ```
  `K_BTN_LEFT_IS_BTNA` wird beim Hardware-Bring-up einmalig geprüft und bei Bedarf invertiert.
- **Touch (`CST820B`):**
  - `if (M5.Touch.getCount() > 0)`: Nimmt `auto t = M5.Touch.getDetail(0);`
  - `out.touch_down = t.isPressed();`
  - `out.touch_x = clamp(t.x / 3, 0, 154);`
  - `out.touch_y = clamp(t.y / 3, 0, 154);`
- **Simulations-Modus (`#ifdef SIM_INPUT`, `UC-12`, `US-23`):**
  - Wenn `AppController` im Zustand `Playing` ist:
    - `out.sim_talk = out.btn_left_down;`
    - `out.sim_shout = out.btn_right_down;`
    - In diesem Modus werden `btn_left_down` / `btn_right_down` nicht an den `InputMapper` für Pause
      weitergereicht (Pause in `SIM_INPUT` nur per Touch auf `❚❚`, `US-23` AK1).

### Persistenz (`M5StorageAdapter`, `FR-PS-01`–`06`)

- Verwendet `<Preferences.h>` mit Namespace `"schreihuhn"`.
- Speichert alle Felder aus `SaveData` kompakt oder als Einzel-Keys (`ver`, `coins`, `hiscore`,
  `unlocked`, `selected`, `sound`).
- **Atomizitäts-Garantie bei Kauf (`FR-PS-05`):** Entweder Speichern als einzelnes `putBytes("save", &data, sizeof(SaveData))`
  (ein NVS-Schreibvorgang pro Commit) oder Reihenfolge `unlocked` vor `coins`. Mit `putBytes` +
  `SaveData::sanitize()` beim Laden ist ein konsistenter Zustand auch bei plötzlichem Stromverlust
  gewährleistet.

### Haptik (`M5HapticsAdapter`, `FR-AU-30`)

- Verwendet `M5.Power.setVibration(180)` bei `vibrate(duration_ms)` und schaltet es nach Ablauf von
  `duration_ms` in `update(now_ms)` nicht-blockierend via `M5.Power.setVibration(0)` wieder aus.

---

## 3.5 Debug-Telemetrie (`DEBUG_LOG`)

Wenn mit `-DDEBUG_LOG` kompiliert wird (`UC-12`, `NFR-01`, `NFR-05`), gibt `main.cpp` exakt alle
`1000 ms` über `Serial` (`115200 Baud`) eine Statuszeile aus:

```text
[DBG] lvl=-42.3dB floor=-61.0dB talk=-49.0dB shout=-33.0dB state=TALK fps=31 x=1234 heap=245120
```

- `lvl`, `floor`, `talk`, `shout`, `state`, `x` entsprechen exakt `UC-12`; `heap` ergänzt den freien
  Heap in Bytes für den 30-Minuten-Stabilitätsnachweis (`NFR-05`).
- In Nicht-Debug-Builds (`[env:stopwatch]`) bleibt diese periodische Ausgabe deaktiviert.
