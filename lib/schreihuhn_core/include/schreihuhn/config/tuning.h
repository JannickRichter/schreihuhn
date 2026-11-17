#pragma once

#include <array>
#include <cstdint>

namespace schreihuhn::config {

// --- Physics & Movement (spec/05, 5.3) ---
inline constexpr float K_GRAVITY       = 600.0f;       // px/s^2
inline constexpr float K_MAX_FALL      = 260.0f;       // px/s
inline constexpr float K_WALK_MIN      = 35.0f;        // px/s (at TALK intensity = 0)
inline constexpr float K_WALK_MAX      = 60.0f;        // px/s (at TALK intensity = 1)
inline constexpr float K_GROUND_ACCEL  = 400.0f;       // px/s^2
inline constexpr float K_GROUND_DECEL  = 600.0f;       // px/s^2
inline constexpr float K_JUMP_VY       = 200.0f;       // px/s (upward initial velocity)
inline constexpr float K_AIR_VX        = 70.0f;        // px/s (airborne horizontal speed without flapping)
inline constexpr float K_FLAP_VX       = 85.0f;        // px/s (airborne horizontal speed while flapping)
inline constexpr float K_FLAP_MAX_FALL = 35.0f;        // px/s (max downward speed while flapping)
inline constexpr float K_FLAP_STAMINA  = 0.8f;         // s (flap stamina per airborne phase)
inline constexpr float K_COYOTE        = 0.08f;        // s (coyote time after leaving ledge)
inline constexpr float K_PHYS_DT       = 1.0f / 60.0f; // s (fixed physics timestep)

// --- Hitbox, Start, Camera & Death (spec/05, 5.3-5.5) ---
inline constexpr float   K_HITBOX_W        = 10.0f;    // px (chicken hitbox width)
inline constexpr float   K_HITBOX_H        = 13.0f;    // px (chicken hitbox height)
inline constexpr float   K_VOID_Y          = 155.0f;   // px (hitbox top death threshold)
inline constexpr float   K_CAMERA_PLAYER_X = 50.0f;    // px (fixed screen x of chicken)
inline constexpr double  K_START_X         = 24.0;     // px (chicken spawn x on start island)
inline constexpr float   K_START_ISLAND_W  = 96.0f;    // px (start island width)
inline constexpr float   K_DYING_MAX_SEC   = 0.8f;     // s (max fall animation before Game Over, spec/05 <= 1.0 s)

// --- World, Levels & Bridges (spec/05, 5.5) ---
inline constexpr float K_TILE_SIZE        = 8.0f;      // px (world tile grid)
inline constexpr float K_LEVEL_0          = 104.0f;    // px (base ground y, Stage 1)
inline constexpr float K_LEVEL_UP1        = 96.0f;     // px (Stage 2)
inline constexpr float K_LEVEL_DOWN1      = 112.0f;    // px (Stage 2)
inline constexpr float K_LEVEL_UP2        = 88.0f;     // px (Stage 3)
inline constexpr float K_LEVEL_DOWN2      = 120.0f;    // px (Stage 3)
inline constexpr float K_BRIDGE_SEG_W     = 8.0f;      // px (bridge segment width)
inline constexpr float K_BRIDGE_SEG_H     = 4.0f;      // px (bridge segment thickness)
inline constexpr float K_BRIDGE_STABILITY = 0.7f;      // s (standing duration before collapse)
inline constexpr float K_BRIDGE_SHAKE     = 0.35f;     // s (remaining stability when shake starts)
inline constexpr float K_BRIDGE_FALL_SEC  = 0.5f;      // s (bridge fall animation duration)
inline constexpr float K_ISLAND_COLL_BOT  = 200.0f;    // px (island collider bottom y)
inline constexpr double K_LOOKAHEAD_PX    = 320.0;     // px (min generated world ahead of screen right)
inline constexpr double K_PRUNE_BEHIND_PX = 32.0;      // px (prune world elements left of camera_x - 32)
inline constexpr float K_LANDING_ZONE_W   = 24.0f;     // px (min flat landing zone after challenge chunk)
inline constexpr float K_OBST_MARGIN_W    = 16.0f;     // px (min flat ground before/after obstacle)
inline constexpr uint32_t K_STAGE2_SCORE  = 100;       // m (Stage 2 start)
inline constexpr uint32_t K_STAGE3_SCORE  = 300;       // m (Stage 3 start)
inline constexpr float K_SCORE_HIGHLIGHT_S = 1.0f;     // s (gold score highlight on beating highscore)

// --- Microphone & Audio (spec/07, 7.1-7.2) ---
inline constexpr uint32_t K_MIC_RATE         = 16000;  // Hz (sample rate)
inline constexpr uint16_t K_MIC_FRAME_MS     = 20;     // ms (analysis window = 320 samples)
inline constexpr bool     K_DC_FILTER        = true;   // DC offset removal active
inline constexpr float    K_SMOOTH_ATTACK    = 0.6f;   // EMA factor when level rises
inline constexpr float    K_SMOOTH_RELEASE   = 0.25f;  // EMA factor when level falls
inline constexpr uint16_t K_CALIB_MS         = 1500;   // ms (ambient noise calibration duration)
inline constexpr float    K_FLOOR_MIN_DB     = -70.0f; // dBFS (min clamped noise floor)
inline constexpr float    K_FLOOR_MAX_DB     = -40.0f; // dBFS (max clamped noise floor)
inline constexpr float    K_TALK_OFFSET_DB   = 12.0f;  // dB (above noise floor)
inline constexpr float    K_SHOUT_OFFSET_DB  = 28.0f;  // dB (above noise floor)
inline constexpr float    K_SHOUT_ABS_MIN_DB = -30.0f; // dBFS (minimum absolute shout threshold)
inline constexpr float    K_SHOUT_MIN_GAP_DB = 10.0f;  // dB (shout >= talk + 10 dB invariant)
inline constexpr float    K_HYSTERESIS_DB    = 3.0f;   // dB (hysteresis when leaving a state)
inline constexpr uint8_t  K_SHOUT_MIN_FRAMES = 2;      // frames (consecutive frames >= shout_db)
inline constexpr uint8_t  K_MIC_GAIN         = 1;      // hardware gain level (tune on hardware)
inline constexpr uint16_t K_GATE_TAIL_MS     = 150;    // ms (gating tail after sound ends)
inline constexpr uint8_t  K_VOLUME           = 40;     // % (0..100 default speaker volume)

// --- UI, Display, Timing & Haptics (spec/06, spec/08, spec/10) ---
inline constexpr int      K_LOGICAL_SIZE      = 155;   // px (155x155 logical canvas)
inline constexpr int      K_DISPLAY_SCALE     = 3;     // factor (465x465 centered on 466x466)
inline constexpr int      K_SAFE_CENTER_X     = 77;    // px (round display safe circle center x)
inline constexpr int      K_SAFE_CENTER_Y     = 77;    // px (round display safe circle center y)
inline constexpr int      K_SAFE_RADIUS       = 72;    // px (round display safe circle radius)
inline constexpr uint16_t K_SPLASH_MS         = 1000;  // ms (minimal boot chicken icon duration)
inline constexpr uint16_t K_GO_BANNER_MS      = 500;   // ms ("LOS!" display duration)
inline constexpr uint16_t K_RESUME_MS         = 1000;  // ms (resume countdown after pause)
inline constexpr uint16_t K_LONGPRESS_MS      = 600;   // ms (long button press threshold)
inline constexpr uint16_t K_TAP_MAX_MS        = 500;   // ms (max touch tap duration)
inline constexpr int16_t  K_TAP_MAX_MOVE_PX   = 12;    // px (max touch movement for valid tap)
inline constexpr uint16_t K_SCREEN_LOCKOUT_MS = 250;   // ms (input lockout after screen switch)
inline constexpr uint16_t K_GAMEOVER_LOCK_MS  = 1000;  // ms (input lockout on Game Over screen)
inline constexpr uint8_t  K_BRIGHTNESS        = 60;    // % (display brightness)
inline constexpr bool     K_BTN_LEFT_IS_BTNA  = true;  // hardware mapping left button == BtnA
inline constexpr uint16_t K_HAPTIC_BUY_MS     = 120;   // ms (vibration on purchase)
inline constexpr uint16_t K_HAPTIC_DEATH_MS   = 200;   // ms (vibration on game over)

// --- Persistence & Shop (spec/09) ---
inline constexpr uint8_t  K_SAVE_SCHEMA  = 1;          // schema version
inline constexpr uint32_t K_COINS_MAX    = 999999;     // coins (max coins & highscore)
inline constexpr uint32_t K_PRICE_GOLDIE = 50;         // coins
inline constexpr uint32_t K_PRICE_NINJA  = 150;        // coins

// --- Chunk Weights for MapGenerator (spec/05 5.6 & spec/14 14.2) ---
// Order matches ChunkType (16 entries):
// FLAT, GAP_S, GAP_M, GAP_L, OBST_LOW, OBST_HIGH, OBST_TOWER,
// BRIDGE_S, BRIDGE_L, STEP, COMBO_OG, COMBO_BO,
// OBST_UNDER, OBST_WIDE, OBST_STAIR, COMBO_UO
inline constexpr std::array<uint8_t, 16> K_CHUNK_WEIGHTS_S1 = {
    4, 4, 0, 0, 3, 0, 0, 3, 0, 0, 0, 0, 2, 0, 0, 0};
inline constexpr std::array<uint8_t, 16> K_CHUNK_WEIGHTS_S2 = {
    2, 2, 3, 1, 2, 2, 0, 2, 2, 2, 0, 0, 2, 2, 2, 1};
inline constexpr std::array<uint8_t, 16> K_CHUNK_WEIGHTS_S3 = {
    1, 1, 2, 3, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2};

// --- Varied Obstacles & Dynamic Biomes (spec/14) ---
inline constexpr float    K_OVERHANG_CLEAR_PX   = 20.0f; // px (ground clearance below Overhang; > K_HITBOX_H=13)
inline constexpr uint8_t  K_REG_BIOME_COUNT     = 6u;    // biomes (Greenland..Sand; Candy is Perk-only)
inline constexpr uint32_t K_BIOME_MIN_M         = 45u;   // m (minimum distance between biome changes)
inline constexpr uint32_t K_BIOME_MAX_M         = 75u;   // m (maximum distance between biome changes)
inline constexpr float    K_BIOME_TRANSITION_S  = 1.4f;  // s (Bayer-dither atmosphere crossfade duration)
inline constexpr float    K_BIOME_BANNER_S      = 1.5f;  // s (HUD biome banner duration)
inline constexpr uint8_t  K_AMBIENT_SNOW_COUNT  = 20u;   // particles (Snow biome falling flakes)
inline constexpr uint8_t  K_AMBIENT_LAVA_COMETS = 5u;    // comets (Lava biome background comets)
inline constexpr uint8_t  K_AMBIENT_LAVA_EMBERS = 10u;   // sparks (Lava biome rising embers)
inline constexpr uint8_t  K_AMBIENT_RAIN_COUNT  = 12u;   // droplets (subtle Jungle rain droplets)
inline constexpr uint8_t  K_AMBIENT_CANDY_COUNT = 18u;   // sparkles (Candy biome floating stars)

// --- White-Packet Flight Perk & Candy Mode (spec/14, 14.5) ---
inline constexpr float    K_PERK_DURATION_S     = 6.5f;   // s (active perk flight + Candy biome duration)
inline constexpr float    K_PERK_WARN_S         = 1.2f;   // s (HUD flash warning before expiry)
inline constexpr float    K_PERK_TRANSITION_S   = 0.45f;  // s (fast crossfade into Candy biome)
inline constexpr uint32_t K_PERK_FIRST_MIN_M    = 26u;    // m (earliest spawn distance from run start)
inline constexpr uint32_t K_PERK_COOLDOWN_M     = 60u;    // m (min distance between perk packet spawns)
inline constexpr float    K_PERK_JUMP_VY        = 240.0f; // px/s (stronger jump impulse during perk)
inline constexpr float    K_PERK_FLOAT_VY       = -95.0f; // px/s (upward lift velocity at Talk intensity=0)
inline constexpr float    K_PERK_CLIMB_VY       = -165.0f;// px/s (climb velocity at Talk intensity=1 / Shout)
inline constexpr float    K_PERK_FLY_VX         = 90.0f;  // px/s (airborne horizontal speed during perk)
inline constexpr float    K_PERK_GRAVITY        = 180.0f; // px/s^2 (reduced glide gravity during perk)
inline constexpr float    K_PERK_MAX_FALL       = 50.0f;  // px/s (gentle glide fall cap during perk)
inline constexpr float    K_PERK_CEILING_Y      = 18.0f;  // px (upper screen barrier y during perk flight)

} // namespace schreihuhn::config
