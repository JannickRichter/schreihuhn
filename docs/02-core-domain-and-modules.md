# 02 – Core-Domäne & Module

Alle hier beschriebenen Typen, Schnittstellen und Algorithmen liegen im Namespace `schreihuhn`
unter `lib/schreihuhn_core/`. Sie verwenden ausschließlich Standard-C++17-Header (`<cstdint>`,
`<cstddef>`, `<cmath>`, `<algorithm>`, `<array>`, `<cstring>`) und dürfen **keine** dynamische
Speicherallokation im Frame-Loop ausführen (`NFR-05`, `NFR-06`).

---

## 2.1 Zentrale Tuning-Konstanten (`config/tuning.h`)

Alle Tuning-Werte aus `/spec` werden als `inline constexpr`-Konstanten in
`lib/schreihuhn_core/include/schreihuhn/config/tuning.h` definiert (`NFR-09`, `AGENTS.md`).
Jede Konstante trägt ihre Einheit im Kommentar.

```cpp
#pragma once
#include <array>
#include <cstdint>

namespace schreihuhn::config {

// --- Physik & Bewegung (spec/05, 5.3) ---
inline constexpr float K_GRAVITY       = 600.0f;       // px/s^2
inline constexpr float K_MAX_FALL      = 260.0f;       // px/s
inline constexpr float K_WALK_MIN      = 35.0f;        // px/s (bei intensity = 0)
inline constexpr float K_WALK_MAX      = 60.0f;        // px/s (bei intensity = 1)
inline constexpr float K_GROUND_ACCEL  = 400.0f;       // px/s^2
inline constexpr float K_GROUND_DECEL  = 600.0f;       // px/s^2
inline constexpr float K_JUMP_VY       = 200.0f;       // px/s (nach oben)
inline constexpr float K_AIR_VX        = 70.0f;        // px/s (ohne Flattern)
inline constexpr float K_FLAP_VX       = 85.0f;        // px/s (beim Flattern)
inline constexpr float K_FLAP_MAX_FALL = 35.0f;        // px/s (max. Sinkgeschwindigkeit beim Flattern)
inline constexpr float K_FLAP_STAMINA  = 0.8f;         // s (pro Luftphase)
inline constexpr float K_COYOTE        = 0.08f;        // s (Coyote-Time nach Kantenverlust)
inline constexpr float K_PHYS_DT       = 1.0f / 60.0f; // s (fester Physik-Zeitschritt)

// --- Hitbox, Start & Kamera (spec/05, 5.3–5.5) ---
inline constexpr float   K_HITBOX_W        = 10.0f;    // px (Huhn-Hitbox Breite)
inline constexpr float   K_HITBOX_H        = 13.0f;    // px (Huhn-Hitbox Höhe)
inline constexpr float   K_VOID_Y          = 155.0f;   // px (untere Todesschwelle für Hitbox-Oberkante)
inline constexpr float   K_CAMERA_PLAYER_X = 50.0f;    // px (feste Bildschirm-X-Position des Huhns)
inline constexpr double  K_START_X         = 24.0;     // px (Startposition des Huhns)
inline constexpr float   K_START_ISLAND_W  = 96.0f;    // px (Breite der Startinsel)

// --- Welt & Brücken (spec/05, 5.5) ---
inline constexpr float K_LEVEL_0          = 104.0f;    // px (Basis-Bodenniveau, Stufe 1)
inline constexpr float K_LEVEL_UP1        = 96.0f;     // px (Stufe 2)
inline constexpr float K_LEVEL_DOWN1      = 112.0f;    // px (Stufe 2)
inline constexpr float K_LEVEL_UP2        = 88.0f;     // px (Stufe 3)
inline constexpr float K_LEVEL_DOWN2      = 120.0f;    // px (Stufe 3)
inline constexpr float K_BRIDGE_STABILITY = 0.7f;      // s (Standzeit pro Segment bis Einsturz)
inline constexpr float K_BRIDGE_SHAKE     = 0.35f;     // s (Restzeit ab der Segment wackelt)

// --- Mikrofon & Audio (spec/07, 7.1–7.2) ---
inline constexpr uint32_t K_MIC_RATE         = 16000;  // Hz (Abtastrate)
inline constexpr uint16_t K_MIC_FRAME_MS     = 20;     // ms (Analyse-Fensterlänge = 320 Samples)
inline constexpr bool     K_DC_FILTER        = true;   // DC-Offset-Entfernung aktiv
inline constexpr float    K_SMOOTH_ATTACK    = 0.6f;   // EMA-Faktor steigend
inline constexpr float    K_SMOOTH_RELEASE   = 0.25f;  // EMA-Faktor fallend
inline constexpr uint16_t K_CALIB_MS         = 1500;   // ms (Grundrausch-Messdauer)
inline constexpr float    K_FLOOR_MIN_DB     = -70.0f; // dBFS (Untergrenze Grundrauschen)
inline constexpr float    K_FLOOR_MAX_DB     = -40.0f; // dBFS (Obergrenze Grundrauschen)
inline constexpr float    K_TALK_OFFSET_DB   = 12.0f;  // dB (über Grundrauschen)
inline constexpr float    K_SHOUT_OFFSET_DB  = 28.0f;  // dB (über Grundrauschen)
inline constexpr float    K_SHOUT_ABS_MIN_DB = -30.0f; // dBFS (Mindestschwelle für Schreien)
inline constexpr float    K_HYSTERESIS_DB    = 3.0f;   // dB (Hysterese beim Verlassen)
inline constexpr uint8_t  K_SHOUT_MIN_FRAMES = 2;      // Frames (min. aufeinanderfolgend über shout_db)
inline constexpr uint8_t  K_MIC_GAIN         = 1;      // Hardware-Gain-Stufe (M5Unified Mic config; auf Hardware tunen)
inline constexpr uint16_t K_GATE_TAIL_MS     = 150;    // ms (Gating-Nachlauf nach Soundende)
inline constexpr uint8_t  K_VOLUME           = 40;     // % (0..100, Standard-Lautstärke)

// --- UI, Display & Haptik (spec/06, 6.1; spec/10) ---
inline constexpr uint16_t K_LONGPRESS_MS      = 600;   // ms (Langdruck-Schwelle Tasten)
inline constexpr uint16_t K_SCREEN_LOCKOUT_MS = 250;   // ms (Eingabesperre nach Screenwechsel)
inline constexpr uint16_t K_GAMEOVER_LOCK_MS  = 1000;  // ms (Eingabesperre im Game Over)
inline constexpr uint8_t  K_BRIGHTNESS        = 60;    // % (Display-Helligkeit)
inline constexpr bool     K_BTN_LEFT_IS_BTNA  = true;  // Zuordnung Taste links/rechts auf Hardware

// --- Persistenz & Shop (spec/09) ---
inline constexpr uint32_t K_COINS_MAX    = 999999;     // Coins (Obergrenze Coins & Highscore)
inline constexpr uint32_t K_PRICE_GOLDIE = 50;         // Coins
inline constexpr uint32_t K_PRICE_NINJA  = 150;        // Coins

// --- Chunk-Gewichtstabelle für MapGenerator (spec/05, 5.6) ---
// Reihenfolge entspricht ChunkType:
// FLAT, GAP_S, GAP_M, GAP_L, OBST_LOW, OBST_HIGH, OBST_TOWER,
// BRIDGE_S, BRIDGE_L, STEP, COMBO_OG, COMBO_BO
inline constexpr std::array<uint8_t, 12> K_CHUNK_WEIGHTS_S1 = {4, 4, 0, 0, 3, 0, 0, 3, 0, 0, 0, 0};
inline constexpr std::array<uint8_t, 12> K_CHUNK_WEIGHTS_S2 = {2, 2, 3, 1, 2, 3, 0, 2, 2, 2, 0, 0};
inline constexpr std::array<uint8_t, 12> K_CHUNK_WEIGHTS_S3 = {1, 1, 2, 3, 1, 2, 2, 1, 3, 2, 2, 2};

} // namespace schreihuhn::config
```

---

## 2.2 Basis-Typen, Präzisionsmodell & Ringpuffer (`core/`)

### Koordinaten- & Präzisionsmodell (`FR-GP-37`, `AT-15`)

- **Horizontale Weltkoordinaten (`world_x`, `camera_x`, Element-`x`)** verwenden `double`. Ein `double`
  löst Subpixel bis weit über $10^9\text{ px}$ exakt auf (`float` würde bereits bei ~100.000 px
  sichtbar stottern). Damit ist `FR-GP-37` (≥ 1.000.000 px) und `AT-15` (100 km = 800.000 px)
  präzisionsverlustfrei erfüllt.
- **Vertikale Koordinaten (`y` in `0..155`)** und Geschwindigkeiten (`vx`, `vy`) verwenden `float`.
- **Rendering-Umrechnung:** `int screen_x = static_cast<int>(std::lround(element_x - camera_x));`
  (`FR-GR-02`: erst beim Zeichnen auf ganze logische Pixel runden).

### Deterministischer PRNG (`math_types.h`, `FR-MP-30`, `AT-13`)

- `XorShift32` mit internem Zustand `uint32_t state_` (falls `seed == 0`, Initialisierung mit
  `0x9E3779B9u`). Bietet `uint32_t nextU32()`, `int32_t rangeInt(int32_t min_inc, int32_t max_inc)`
  und `float nextFloat01()`. Identisch auf ESP32 und Host.

### Statischer Ringpuffer (`ring_buffer.h`, `NFR-06`)

```cpp
template <typename T, std::size_t Capacity>
class StaticRingBuffer {
public:
    bool pushBack(const T& item);   // false falls voll
    void popFront();                // entfernt ältestes Element (O(1))
    void clear();
    std::size_t size() const;
    bool empty() const;
    bool full() const;
    T& operator[](std::size_t idx); // 0 = ältestes Element
    const T& operator[](std::size_t idx) const;
};
```

Kapazitäten für eine sichtbare Breite von `155 px` + Lookahead `320 px` + Nachlauf `32 px` (`≈ 507 px`
Gesamtfester):
- `Islands`: `Capacity = 32` (min. Breite `24 px` → max. ~21 sichtbar)
- `BridgeSegments`: `Capacity = 64` (Breite `8 px` → bei Bedarf Puffer für mehrere Brücken)
- `Obstacles`: `Capacity = 32`
- `Coins`: `Capacity = 128`
- `Particles`: `Capacity = 32` (Glitzer / optional Staub/Federn)

---

## 2.3 HAL-Schnittstellen (`hal/`)

| Interface / Typ | Methoden / Felder | Zweck |
|-----------------|-------------------|-------|
| `IStorage` | `bool load(SaveData& out)`, `bool save(const SaveData& in)`, `bool reset()` | Abstraktion für ESP32 `Preferences` (`schreihuhn`) bzw. `MockStorage` |
| `IAudioOut` | `void playSound(SoundId id)`, `void setSoundEnabled(bool on)`, `bool isSoundEnabled() const` | Löst Sound-Wiedergabe aus; auf ESP32 startet dies auch das Gating |
| `IHaptics` | `void vibrate(uint16_t duration_ms)` | Kurze Vibration bei Kauf (`120 ms`) / Game Over (`200 ms`) (`FR-AU-30`) |
| `PlatformInput` | `bool btn_left_down`, `bool btn_right_down`, `bool touch_down`, `int16_t touch_x`, `int16_t touch_y`, `bool sim_talk`, `bool sim_shout` | Rohzustand pro Frame (Koordinaten bereits in `0..154` logischen px) |
| `Canvas155` | `std::array<uint8_t, 155 * 155> pixels`, `clear(uint8_t color)`, `setPixel(int x, int y, uint8_t color)`, `fillRect(...)`, `drawDitherRect(...)` | Zielpuffer des Cores (`155 × 155 × 1 Byte ≈ 24 KB`) |

---

## 2.4 Mikrofon-Klassifizierung (`audio/mic_classifier.h`)

### Zustände & Datenmodell

```cpp
enum class VoiceState : uint8_t { Still, Talk, Shout };

struct MicClassification {
    VoiceState state = VoiceState::Still;
    float intensity  = 0.0f;   // 0..1 innerhalb TALK (FR-AU-06)
    float raw_db     = -90.0f; // letzter ungeglätteter Frame-Pegel
    float smoothed_db = -90.0f;
    float floor_db   = -60.0f;
    float talk_db    = -48.0f;
    float shout_db   = -30.0f;
    bool  calibrated = false;
    bool  gated      = false;
};
```

### Verarbeitungs-Pipeline (`FR-AU-01`…`10`)

1. **`processSamples(const int16_t* samples, size_t count, float dt_sec)`:**
   - Bei `K_DC_FILTER == true`: Mittelwert $\mu = \frac{1}{N}\sum s_i$ über das Frame berechnen und
     von jedem Sample subtrahieren (`s_i - mu`).
   - RMS berechnen: $\text{rms} = \sqrt{\frac{1}{N}\sum (s_i - \mu)^2}$.
   - `raw_db = (rms <= 1.0f) ? -90.0f : std::max(-90.0f, 20.0f * std::log10(rms / 32768.0f))`.
   - Übergabe an `processFrameDb(raw_db, dt_sec)`.
2. **Kalibrierung (`beginCalibration()`, `finishCalibration()`):**
   - Während `Countdown` speichert `processFrameDb()` bis zu `96` Frame-Pegel (`K_CALIB_MS / K_MIC_FRAME_MS = 75` Frames) in einem statischen `std::array<float, 96>`.
   - Bei `finishCalibration()`:
     - Kopie sortieren (`std::sort`) → Median entnehmen. Falls 0 Frames vorlagen (z. B. Fallback), `-60.0f`.
     - `floor_db = clamp(median, K_FLOOR_MIN_DB, K_FLOOR_MAX_DB)`.
     - `talk_db = floor_db + K_TALK_OFFSET_DB`.
     - `shout_db = std::max(floor_db + K_SHOUT_OFFSET_DB, K_SHOUT_ABS_MIN_DB)`.
     - Sicherheitsinvariante (`FR-AU-04`): `if (shout_db < talk_db + 10.0f) shout_db = talk_db + 10.0f;`.
     - `smoothed_db = floor_db`, `state = VoiceState::Still`, `calibrated = true`.
3. **Gating (`triggerSoundGate(uint16_t sound_duration_ms)`):**
   - Setzt `gate_remaining_ms_ = std::max<uint32_t>(gate_remaining_ms_, sound_duration_ms + K_GATE_TAIL_MS)`.
   - Solange `gate_remaining_ms_ > 0`: `gate_remaining_ms_` um `dt_ms` reduzieren, Frame verwerfen.
     `smoothed_db` und `state` bleiben unverändert (`FR-AU-10`); `shout_candidate_frames_` wird auf `0`
     zurückgesetzt, sodass während des Gatings kein neuer Übergang nach `SHOUT` entstehen kann.
4. **Glättung & Hysterese (`FR-AU-02`, `FR-AU-05`, `FR-AU-06`):**
   - `alpha = (raw_db > smoothed_db) ? K_SMOOTH_ATTACK : K_SMOOTH_RELEASE;`
   - `smoothed_db += alpha * (raw_db - smoothed_db);`
   - Schrei-Kandidaten-Zähler: `if (smoothed_db >= shout_db) ++shout_frames_; else shout_frames_ = 0;`
   - Zustandsautomat:
     - Aus `Still`: wenn `shout_frames_ >= K_SHOUT_MIN_FRAMES` → `Shout`; sonst wenn `smoothed_db >= talk_db` → `Talk`.
     - Aus `Talk`: wenn `shout_frames_ >= K_SHOUT_MIN_FRAMES` → `Shout`; sonst wenn `smoothed_db < talk_db - K_HYSTERESIS_DB` → `Still`.
     - Aus `Shout`: wenn `smoothed_db < shout_db - K_HYSTERESIS_DB` → Übergang zu `(smoothed_db >= talk_db - K_HYSTERESIS_DB) ? Talk : Still`.
   - `intensity = (state == VoiceState::Talk) ? clamp((smoothed_db - talk_db) / (shout_db - talk_db), 0.0f, 1.0f) : 0.0f`.

---

## 2.5 Physik, Kollision & Brücken (`game/`)

### Schrittfolge pro `K_PHYS_DT`-Tick (`1/60 s`)

1. **Input auf `PlayerState` anwenden (`FR-GP-01`…`15`):**
   - **Sprungbedingung:** `(player.on_ground || player.coyote_timer > 0.0f) && voice.state == VoiceState::Shout`
     → `player.vy = -K_JUMP_VY`, `player.vx = K_AIR_VX`, `player.on_ground = false`,
     `player.coyote_timer = 0.0f`, `player.is_flapping = false`.
   - **Am Boden (`player.on_ground` nach obiger Prüfung):**
     - `VoiceState::Still`: `player.vx = moveToward(player.vx, 0.0f, K_GROUND_DECEL * dt)`.
       (Von `60 px/s` auf `0` bei `600 px/s²` = `100 ms`, erfüllt `FR-GP-03` ≤ 150 ms).
     - `VoiceState::Talk`: `target = K_WALK_MIN + voice.intensity * (K_WALK_MAX - K_WALK_MIN)`.
       Falls `player.vx < target`: Schritt `K_GROUND_ACCEL * dt`, sonst `K_GROUND_DECEL * dt`.
   - **In der Luft (`!player.on_ground`):**
     - `player.vy = std::min(player.vy + K_GRAVITY * dt, K_MAX_FALL)`.
     - Falls `voice.state == VoiceState::Shout && player.stamina > 0.0f`:
       `player.vx = K_FLAP_VX`; `player.is_flapping = true`.
       Falls `player.vy >= 0.0f` (sinkend, `FR-GP-13`): `player.stamina = std::max(0.0f, player.stamina - dt)`;
       falls `player.stamina > 0.0f`: `player.vy = std::min(player.vy, K_FLAP_MAX_FALL)`.
     - Sonst (`FR-GP-11`, `FR-GP-14`): `player.is_flapping = false`. (Falls zuvor geflattert wurde und
       Ausdauer leer ist, `player.vx = K_AIR_VX`).

2. **Horizontale Bewegung & AABB-Kollision (`FR-GP-20`, `FR-GP-21`):**
   - `player.world_x += static_cast<double>(player.vx * dt);`
   - Huhn-Hitbox: `left = player.world_x`, `right = left + K_HITBOX_W`, `top = player.y`, `bottom = player.y + K_HITBOX_H`.
   - Prüfe gegen alle festen Collider (`Island` `[x, x+w]×[y, 200]`, **intakte** `BridgeSegment` `[x, x+8]×[y, y+4]`,
     `Obstacle` `[x, x+w]×[y, y+h]`):
     - Wenn vertikale Überlappung (`bottom > col_top + 0.01f && top < col_bottom - 0.01f`) und
       `right > col_left && left < col_right`:
       `player.world_x = col_left - K_HITBOX_W; player.vx = 0.0f;`

3. **Vertikale Bewegung & Anti-Tunneling (`FR-GP-20`, `FR-GP-22`, `FR-GP-24`, `AT-09`):**
   > **Kritischer Punkt:** Bei `K_MAX_FALL = 260 px/s` beträgt der Fallweg je Tick `4,33 px`, während
   > ein `BridgeSegment` nur `4,0 px` dick ist. Eine naive Endpunkt-Überlappungsprüfung könnte
   > dünne Brückensegmente durchtunneln. Deshalb wird eine Swept-Top-Surface-Prüfung verwendet:
   - `float prev_bottom = player.y + K_HITBOX_H;`
   - `player.y += player.vy * dt;`
   - `float new_bottom = player.y + K_HITBOX_H;`
   - `bool landed = false;`
   - **Wenn `player.vy >= 0.0f`:** Suche unter allen festen Collidern mit horizontaler Überlappung
     (`right > col_left && left < col_right`) den kleinsten `col_top`, für den gilt:
     `prev_bottom <= col_top + 0.5f && new_bottom >= col_top`.
     Falls gefunden:
     `player.y = col_top - K_HITBOX_H; player.vy = 0.0f; landed = true;`
   - **Wenn `player.vy < 0.0f`:** Bei Überlappung mit einem Collider (`bottom > col_top && top < col_bottom`
     und horizontaler Überlappung): `player.y = col_bottom; player.vy = 0.0f;` (`FR-GP-22`).
   - **Landung / Coyote-Time (`FR-GP-10`, `FR-GP-13`):**
     - Wenn `landed`: `player.on_ground = true; player.stamina = K_FLAP_STAMINA; player.coyote_timer = K_COYOTE; player.is_flapping = false;`
     - Sonst: `player.on_ground = false; player.coyote_timer = std::max(0.0f, player.coyote_timer - dt);`

4. **Brücken-Stabilität (`FR-MP-11`…`16`):**
   - Wenn `player.on_ground`: Für jedes `BridgeSegment` mit `intact == true`:
     - Falls `std::fabs(seg.y - (player.y + K_HITBOX_H)) < 0.5f` und `right > seg.x && left < seg.x + 8.0`:
       `seg.stability -= dt;`
       Wenn `seg.stability <= 0.0f`: `seg.intact = false; seg.fall_vy = 0.0f;` (falls Hindernis auf
       diesem Segment steht: `obstacle.broken = true` / fällt mit ab, `spec/05` 5.5).
   - Für bereits gebrochene Brückensegmente (`!seg.intact`): `seg.fall_vy += K_GRAVITY * dt; seg.y += seg.fall_vy * dt;`
     für die ca. `0,5 s` Fall-Animation (`FR-MP-13`).

5. **Coins, Kamera, Score & Void-Tod (`FR-GP-30`, `FR-GP-35`, `FR-GP-40`, `FR-GP-50`):**
   - Coin-Hitbox (`6×6` zentriert in `8×8`, also `[coin.x+1, coin.x+7] × [coin.y+1, coin.y+7]`):
     Bei AABB-Überlappung mit Huhn-Hitbox: `coin.collected = true; ++run_coins; spawnSparkle(coin.x, coin.y); audio->playSound(SoundId::Coin);`.
   - `max_world_x = std::max(max_world_x, player.world_x);`
   - `camera_x = std::max(camera_x, player.world_x - static_cast<double>(K_CAMERA_PLAYER_X));`
   - `score_m = static_cast<uint32_t>(std::floor((max_world_x - K_START_X) / 8.0));`
   - Wenn `player.y > K_VOID_Y`: Wechsel in Run-Zustand `Dying` (`FR-GP-30`, `FR-GP-31`, max. `1.0 s` Fall-Timer bis `Ended`).

---

## 2.6 Map-Generator & Lösbarkeits-Garantie (`world/`)

### Datenstrukturen (`world_types.h`)

- `Island`: `double x; float w; float y;` (Breite `24..120`, Vielfaches von `8`, Kollision nach unten bis `200`).
- `BridgeSegment`: `double x; float y; float stability; bool intact; float fall_vy;`
- `Obstacle`: `double x; float y; float w; float h; ObstacleType type; bool falling; float fall_vy;` (`HayBale 8×8`, `Fence 8×16`, `Stone 16×16`, `Tower 8×24`).
- `Coin`: `double x; float y; bool collected;`

### `SolvabilityChecker` (`solvability.h`, `FR-MP-36`)

Der `SolvabilityChecker` verwendet exakt dieselben Konstanten und Update-Gleichungen wie `Physics`:

- `simulateJumpDistance(float delta_y, bool with_flap)`:
  Startet bei `x = 0, y = 0` auf dem Boden mit `SHOUT` im ersten Tick (für `with_flap = false`
  danach `STILL`; für `with_flap = true` durchgehend `SHOUT` bis `y >= delta_y` bei `vy > 0`).
  Gibt die erreichte horizontale Distanz `W` sowie optionale Trajektorien-Samples `(x_i, y_i)` zurück.
- Zulässige maximale Lückenbreite (auf `8 px` abgerundet):
  - Ohne Flattern (`GAP_S`, `GAP_M`, `STEP`): `max_gap = floor(0.85f * W_jump(delta_y) / 8.0f) * 8.0f`
  - Mit Flattern (`GAP_L`): `max_gap = floor(0.80f * W_flap(delta_y) / 8.0f) * 8.0f`
- Coin-Bögen (`FR-MP-26` b/c) platzieren Coins entlang der gespeicherten Trajektorien-Samples minus
  halbe Hitbox-Höhe, wodurch jeder Coin nachweislich innerhalb der realen Flugbahn liegt (`FR-MP-27`).

### `MapGenerator` (`map_generator.h`, `FR-MP-30`…`37`)

1. **`reset(uint32_t seed)`:**
   - Initialisiert `XorShift32 rng(seed)`.
   - Erzeugt Startinsel bei `x = 0, w = K_START_ISLAND_W (96), y = K_LEVEL_0 (104)` (`FR-MP-03`).
   - Setzt `cursor_x = 96.0`, `current_level_y = K_LEVEL_0`, `repeat_count = 0`.
2. **`ensureGeneratedUpTo(double target_x)`** (aufgerufen mit `camera_x + 155.0 + 320.0`, `FR-MP-31`):
   - Solange `cursor_x < target_x`:
     - Bestimme Stufe aus `score_at_cursor = (cursor_x - K_START_X) / 8.0`:
       - `< 100`: Stufe 1 (`K_CHUNK_WEIGHTS_S1`)
       - `100..299`: Stufe 2 (`K_CHUNK_WEIGHTS_S2`)
       - `>= 300`: Stufe 3 (`K_CHUNK_WEIGHTS_S3`)
     - Wähle gewichteten `ChunkType` über `rng`, wobei `chunk == last_chunk && repeat_count >= 2`
       ausgeschlossen wird (`FR-MP-35`).
     - Baue Chunk an `cursor_x`:
       - Vor/nach jedem Hindernis mindestens `16 px` freier Boden auf gleicher Höhe (`spec/05` 5.5).
       - Nach jedem Herausforderungs-Chunk (alles außer `FLAT`) folgt mindestens `24 px` flacher
         Boden ohne Hindernis (`FR-MP-33` Landezone).
       - Lückenbreiten werden gegen `SolvabilityChecker::maxGap(...)` geklemmt.
       - Niveauwechsel per `STEP`: `±8 px` in Stufe 2, `±8 / ±16 px` in Stufe 3, stets innerhalb
         `[K_LEVEL_UP2 (88), K_LEVEL_DOWN2 (120)]`.
       - `COMBO_OG`: Insel mit Hindernis, dann `16 px` Anlauf/Landung bzw. `16 px` vor Lücke `GAP_S`
         (so gewählt, dass Hindernis + `16 px` flach vor `GAP_S` liegt, vgl. `spec/05` 5.5 & Chunk-Tabelle).
       - `COMBO_BO`: Brücke mit 5–7 Segmenten (`40–56 px`), Heuballen auf mittlerem Segment (mit ≥ 16 px
         Abstand zu den Brückenenden).
       - Coin-Platzierung gemäß Dichte-Ziel (~1 Coin pro 8–12 m = 64–96 px im Mittel) als Reihe,
         Sprung-Bogen oder Flatter-Reihe (`FR-MP-26`, `FR-MP-27`).
3. **`pruneBefore(double min_x)`** (aufgerufen mit `camera_x - 32.0`, `FR-MP-31`):
   - Entfernt aus `islands`, `bridges`, `obstacles`, `coins` alle Elemente vom Pufferanfang, deren
     rechtes Ende `< min_x` liegt (`O(1)` pro entferntem Element über `popFront()`).

---

## 2.7 Persistenz, Shop, Input-Mapper & App-State-Machine

### Persistenz & Shop (`persistence/save_data.h`, `persistence/shop_logic.h`, `spec/09`)

```cpp
struct SaveData {
    uint8_t  schema_version = 1;
    uint32_t coins          = 0;
    uint32_t highscore      = 0;
    uint8_t  unlocked       = 0b001; // Bit 0 = HENNI, Bit 1 = GOLDIE, Bit 2 = NINJA
    uint8_t  selected       = 0;     // 0..2
    bool     sound_on       = true;

    void sanitize(); // Erzwingt Bereiche, Bit 0 immer 1, selected muss freigeschaltet sein (FR-PS-02/03/06)
};
```

- `ShopLogic`:
  - `uint32_t priceOf(ChickenId id)` → `0`, `K_PRICE_GOLDIE (50)`, `K_PRICE_NINJA (150)`.
  - `bool canAfford(const SaveData& s, ChickenId id)` → `!isUnlocked(s, id) && s.coins >= priceOf(id)`.
  - `bool buy(SaveData& s, ChickenId id, IStorage& storage)` → zieht ab, setzt Bit, setzt
    `selected`, ruft `storage.save(s)` auf (`FR-PS-05`, `FR-SH-03`).

### Input-Mapper (`input/input_mapper.h`, `FR-UI-P4`…`P7`)

```cpp
enum class UIEvent : uint8_t {
    None,
    LeftShort, LeftLong,
    RightShort, RightLong,
    TapStart, TapEnd
};
```

Regeln im `InputMapper`:
1. **Screen-Lockout (`FR-UI-P7`):** Bei `onScreenChanged(uint16_t lockout_ms)` werden alle Events für
   `lockout_ms` (`250 ms`, bei Game Over `1000 ms`) unterdrückt. Zudem wird ein `wait_for_release_`
   Flag gesetzt für Tasten/Touch, die zum Zeitpunkt des Wechsels bereits gedrückt waren – erst nach
   vollständigem Loslassen werden neue Aktionen akzeptiert.
2. **Tasten (`FR-UI-P5`):**
   - Wird bei `>= K_LONGPRESS_MS (600 ms)` Haltedauer ausgelöst → sofort `LeftLong`/`RightLong`
     feuern und `long_fired = true` setzen.
   - Beim Loslassen: nur wenn `!long_fired`, `LeftShort`/`RightShort` feuern.
3. **Touch-Tap (`FR-UI-P4`):**
   - Speichert `press_x, press_y, press_ms` bei `touch_down` Flanke.
   - Beim Loslassen: Wenn `elapsed_ms <= 500` und `|x - press_x| < 10 && |y - press_y| < 10`,
     wird ein gültiger Tap an `(press_x, press_y) -> (release_x, release_y)` gemeldet. Der aktive
     Screen prüft, ob **beide** Punkte in derselben Hitbox liegen.

### App-Zustandsautomat (`app/app_controller.h`, `spec/06` 6.2)

`AppState`: `Splash`, `MainMenu`, `BuyDialog`, `Countdown`, `Playing`, `Paused`, `ResumeCountdown`, `GameOver`.

- Bei jedem Zustandswechsel wird `input_mapper.onScreenChanged(...)` aufgerufen (`1000 ms` für
  `GameOver`, `250 ms` für alle anderen).
- In `MainMenu`: `carousel_index` startet mit `save.selected`; Blättern ändert nur `carousel_index`,
  **noch nicht** `save.selected` (`UC-03` Regel). Erst bei `START` wird `save.selected = carousel_index`
  gesetzt und bei Änderung gespeichert (`FR-PS-04`).
- In `Paused → MENÜ`: `run_session.commitToSave(save, storage)` speichert Coins & Highscore regulär
  (`UC-08` Alternativszenario), danach Wechsel direkt nach `MainMenu`.
- In `Playing → Void-Tod → GameOver`: `run_session.commitToSave(save, storage)` wird **vor** dem
  Übergang nach `GameOver` ausgeführt (`UC-09a`, `FR-UI-43`).
