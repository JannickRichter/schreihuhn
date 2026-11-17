# 04 – Rendering & Assets

Alle Grafiken werden vollständig im Quellcode erzeugt (`RB-05`). Der Core rendert in ein
`155 × 155`-Pixel-Array (`Canvas155`), bei dem jedes Byte einen Palettenindex (`0..15`) enthält.

---

## 4.1 Farbpalette (`render/palette.h`, `spec/08` 8.2)

```cpp
namespace schreihuhn::render {

inline constexpr uint8_t K_TRANSPARENT = 0xFF;

// PICO-8 Palette in RGB565 (Little-Endian/Standard 16-bit; bei Direct-Buffer-Write ggf. Byteswap prüfen)
inline constexpr std::array<uint16_t, 16> K_PALETTE_RGB565 = {
    0x0000, //  0: #000000 Schwarz
    0x194A, //  1: #1D2B53 Dunkelblau
    0x792A, //  2: #7E2553 Dunkelviolett
    0x042A, //  3: #008751 Dunkelgrün
    0xAA86, //  4: #AB5236 Braun
    0x5AB9, //  5: #5F574F Dunkelgrau
    0xC618, //  6: #C2C3C7 Hellgrau
    0xFF9D, //  7: #FFF1E8 Weiß
    0xF809, //  8: #FF004D Rot
    0xFD00, //  9: #FFA300 Orange
    0xFF64, // 10: #FFEC27 Gelb
    0x0726, // 11: #00E436 Grün
    0x2D7F, // 12: #29ADFF Hellblau
    0x83B3, // 13: #83769C Lavendel
    0xFBBA, // 14: #FF77A8 Rosa
    0xFE75  // 15: #FFCCAA Pfirsich
};

} // namespace schreihuhn::render
```

### Abdunkeln / Overlays (`Canvas155::drawDitherOverlay`)

Für `Pause` (`S6`) und `Kaufdialog` (`S3`) wird der Hintergrund mit einem 50-%-Schachbrett-Dither
abgedunkelt (`spec/08` 8.2): Für jedes Pixel `(x, y)` mit `((x + y) & 1) == 0` wird Index `1`
(Dunkelblau `#1D2B53`) bzw. `0` (Schwarz) gesetzt.

---

## 4.2 Sprite-Format & Paletten-Swap (`render/sprites.h`)

### Kompaktes Code-Format

Sprites liegen als `constexpr` Byte-Arrays vor (Werte `0..15` oder `K_TRANSPARENT = 0xFF`),
sodass sie direkt ohne Bit-Entpacken und ohne Heap-Allokation gezeichnet werden können:

- **Huhn (`16 × 16`):**
  - `Idle` (2 Frames), `Walk` (4 Frames), `Jump` (1 Frame), `Flap` (2 Frames), `Fall` (1 Frame).
  - Verwendet logische Farb-Slots im Basis-Sprite:
    - `7` = Primärkörper, `6` = Körperschatten, `8` = Kamm, `9` = Schnabel/Füße, `0` = Auge/Outline, `14` = Wange.
- **Paletten-Swap für die 3 Hühner (`spec/08` 8.4):**
  Beim Zeichnen wird eine 16-Byte-`RemapTable` übergeben (`pixel = remap[raw_pixel]`):
  - **HENNI:** Identitäts-Mapping (`7→7`, `6→6`, `8→8`, `9→9`, `14→7` oder weggelassen).
  - **GOLDIE:** `7→10` (Gelb), `6→9` (Orange), `8→8` (Rot), `9→9` (Orange), `14→14` (Rosa Wange),
    plus 1-px-Glanzpunkt (`7`) auf dem Rücken.
  - **NINJA:** `7→5` (Dunkelgrau), `6→0` (Schwarz), `8→8` (Rot), `9→9` (Orange); zusätzlich wird
    ein `16×16` Stirnband-Overlay (2 Frames flatterndes Bandende, Index `8`) darübergezeichnet.
  - **Gesperrte Vorschau im Menü (`FR-UI-13`):** Silhouette-Mapping (alle Nicht-Transparent-Pixel
    auf Index `1` bzw. `5`) + zentriertes `Lock`-Icon (`9×11`).
- **Welt- & UI-Sprites:**
  - `Coin` (`8 × 8`, 4 Rotations-Frames), `CoinSparkle` (`8 × 8`, 3 Frames).
  - `GrassTile` (`8 × 8`), `DirtTile` (`8 × 8`), `BridgePlank` (`8 × 4`).
  - `HayBale` (`8 × 8`), `Fence` (`8 × 16`), `StoneBlock` (`16 × 16`), `Tower` (`8 × 24`).
  - UI-Icons (`7×7` bis `12×12`): `CoinSmall`, `Lock`, `SpeakerOn`, `SpeakerOff`, `PauseIcon`,
    `ArrowLeft`, `ArrowRight`.

---

## 4.3 Pixel-Font (`render/font5x7.h`, `FR-GR-05`, `FR-UI-P8`)

- Monospace-Bitmap-Font **5 × 7 px** (pro Zeichen `5` Bytes à 7 Bits = 5 Spalten).
- Zeichenabstand `1 px` (effektive Zeichenbreite `6 px` bei Skalierung `1×`, `12 px` bei `2×`).
- Unterstützt:
  - ASCII `A–Z`, `0–9`, Leerzeichen, `.`, `:`, `-`, `+`, `!`, `?`, `/`, `=`.
  - Deutsche Großbuchstaben-Umlaute `Ä`, `Ö`, `Ü` (per einfachem UTF-8-Stepper im `drawText()`-Helper,
    der `0xC3 0x84/0x96/0x9C` direkt auf die drei Umlaut-Glyphen abbildet), sodass die verbindlichen
    Texte aus `spec/06` 6.5 (`MENÜ`, `GEWÄHLT`, `FÜR`) originalgetreu erscheinen.
- **`drawTextOutlined(...)`:** Zeichnet zuerst 4-fach (oder 8-fach) in Farbe `0` (Schwarz, ±1 px)
  und danach in der Zielfarbe (`FR-UI-33` / `NFR-12`).

---

## 4.4 Darstellungsreihenfolge (`WorldRenderer` & `UIRenderer`, `spec/08` 8.5)

In jedem Frame wird exakt diese Reihenfolge auf `Canvas155` eingehalten:

1. **Himmel / Void-Verlauf (`FR-GR-10`):**
   - `y = 0..103`: Hellblau (`12`).
   - `y = 104..134`: Geditherter Übergang von Hellblau (`12`) zu Dunkelblau (`1`).
   - `y = 135..154`: Dunkelblau (`1`) mit vereinzelten Schwarz-Dithers am unteren Rand.
2. **Parallax-Ebenen (`FR-GR-11`):**
   - Wolken (Weiß `7` / Hellgrau `6`), scrollen mit `camera_x * 0.2`, Kachelbreite `160 px`.
   - Ferne Hügel (Lavendel `13`, Schatten `2`/`1`), scrollen mit `camera_x * 0.4`, Kachelbreite `160 px`.
3. **Inseln, Brücken, Hindernisse (`FR-GR-12`–`14`):**
   - Inseln: Oben Grasreihe (`11`/`3`), darunter Erde (`4`) mit Steinchen, nach unten über
     2–3 Kachelreihen verjüngende Spitze mit 1-px-Outline (`0`).
   - Brückensegmente: Falls `stability < K_BRIDGE_SHAKE`, horizontaler Versatz `((tick / 2) & 1) ? 1 : -1`.
   - Hindernisse: Sprite-basiert (`HayBale`, `Fence`, `StoneBlock`, `Tower`).
4. **Coins:** Sinus-Schwebeversatz `round(sin(time * 5 + coin_x * 0.1))` (±1 px, rein optisch, `FR-MP-25`).
5. **Huhn & Partikel:**
   - Sprite-Auswahl gemäß `PlayerState::anim` (`Idle`, `Walk`, `Jump`, `Flap`, `Fall`) an
     `(round(world_x - camera_x) - 3, round(y) - 3)` (da Hitbox `10×13` im `16×16`-Sprite
     horizontal zentriert und unten bündig liegt, `FR-GP-23`).
   - In der Luft: kleiner horizontaler Ausdauer-Balken (`10 × 2 px`) über dem Huhn (`FR-GP-14`).
6. **HUD (`S5`):** Score, Run-Coins, Pause-Symbol, Pegelbalken.
7. **Overlays (`S3`, `S4`, `S6`, `S7`):** Dither-Abdunklung + modale Dialoge/Texte.

---

## 4.5 Verifizierte Screen-Layouts innerhalb des Safe-Kreises (`FR-UI-P1`, `FR-UI-P3`)

Safe-Kreis: Mittelpunkt `(77, 77)`, Radius `72 px` → alle Eckpunkte `(x, y)` jeder Text-/Button-Box
erfüllen $(x - 77)^2 + (y - 77)^2 \le 5184$. Alle Touch-Hitboxen haben mindestens `24 × 20` px.

| Screen | Element | Text / Inhalt | Visuelle Box `(x, y, w, h)` | Touch-Hitbox `(x, y, w, h)` |
|--------|---------|---------------|-----------------------------|-----------------------------|
| **S1 Splash** | Titel | `SCHREIHUHN` (2×) | `(17, 36, 120, 14)` | – (Tap überall) |
| | Huhn | 2× Skaliert (`32×32`) | `(61, 58, 32, 32)` | – |
| | Hinweis | `REDEN = LAUFEN` / `SCHREIEN = SPRINGEN` | zentriert `y = 102`, `y = 112` | – |
| | Fehler | `MIKROFON-FEHLER` (rot, falls nötig) | zentriert `y = 126` | – |
| **S2 Hauptmenü** | Coins | Coin-Icon + Tausenderpunkt-Zahl | `(48, 12, 58, 9)` | – |
| | Highscore | `HI <n> m` | `(46, 24, 62, 8)` | – |
| | Pfeil links `◀` | `ArrowLeft` | `(18, 56, 12, 16)` | `(14, 50, 28, 40)` |
| | Pfeil rechts `▶` | `ArrowRight` | `(125, 56, 12, 16)` | `(113, 50, 28, 40)` |
| | Huhn-Vorschau | 3× Skaliert (`48×48`) | `(53, 39, 48, 48)` | – |
| | Name / Status | `HENNI`/`GOLDIE`/`NINJA`, Status | zentriert `y = 89`, `y = 98` | – |
| | Aktions-Button | `START` / `KAUFEN <p>` / `FEHLEN <n>` | `(39, 107, 77, 20)` | `(39, 107, 77, 20)` |
| | Ton-Symbol | `SpeakerOn` / `SpeakerOff` | `(72, 132, 11, 11)` | `(65, 127, 24, 20)` |
| **S3 Kaufdialog** | Box | `<NAME>` / `FÜR <p> KAUFEN?` | `(24, 52, 107, 60)` | außerhalb = `NEIN` |
| | Button `NEIN` | links | `(28, 86, 44, 20)` | `(28, 86, 44, 20)` |
| | Button `JA` | rechts | `(83, 86, 44, 20)` | `(83, 86, 44, 20)` |
| **S4 Countdown** | Zahl | `3` / `2` / `1` / `LOS!` (2×) | zentriert `y = 60` | – (gesperrt) |
| | Untertitel | `PSST... LEISE!` | zentriert `y = 82` | – |
| **S5 HUD** | Score | `<n> m` | `(52, 12, 50, 12)` | – |
| | Pause | `❚❚` | `(30, 28, 12, 12)` | `(28, 26, 30, 24)` |
| | Run-Coins | Coin-Icon + `<n>` | `(98, 28, 30, 9)` | – |
| | Pegelbalken | 50 × 5 px + 2 Schwellen-Ticks | `(52, 138, 50, 5)` | – |
| **S6 Pause** | Titel / Score | `PAUSE` / `<n> m` | zentriert `y = 44`, `y = 60` | – |
| | Button `MENÜ` | links | `(26, 86, 48, 22)` | `(26, 86, 48, 22)` |
| | Button `WEITER` | rechts | `(81, 86, 48, 22)` | `(81, 86, 48, 22)` |
| **S7 Game Over** | Titel / Hi | `GAME OVER` / `NEUER HIGHSCORE!` | zentriert `y = 28`, `y = 42` | – |
| | Ergebnisse | `<n> m`, `+● <n>`, `GESAMT ● <n>` | zentriert `y = 56, 74, 86` | – |
| | Button `MENÜ` | links | `(26, 104, 48, 22)` | `(26, 104, 48, 22)` |
| | Button `NOCHMAL` | rechts | `(81, 104, 48, 22)` | `(81, 104, 48, 22)` |
