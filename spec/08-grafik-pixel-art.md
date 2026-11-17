# 08 – Grafik & Pixel-Art

## 8.1 Auflösung & Darstellung

| ID | Anforderung |
|----|-------------|
| FR-GR-01 | **Logische Auflösung 155 × 155 px**, Darstellung mit **Faktor 3** (Nearest Neighbor) → 465 × 465 physische px, zentriert auf 466 × 466 (1 px Rand schwarz). |
| FR-GR-02 | Keine Kantenglättung, keine Subpixel-Darstellung: Zeichenpositionen werden auf ganze logische Pixel gerundet (Physik darf intern feiner rechnen). |
| FR-GR-03 | Flimmer-/Tearing-freie Darstellung (z. B. Double-Buffering). Kein sichtbares Zeilenweises Aufbauen. |
| FR-GR-04 | Das runde Display schneidet die Ecken ab: Welt/Hintergrund wird vollflächig gezeichnet, UI hält den Safe-Kreis ein (FR-UI-P1). |
| FR-GR-05 | Schrift: Pixel-Font ca. 5 × 7 px (logisch) in Größe 1× und 2×; zulässig ist eine eingebaute Bibliotheksschrift, sofern sie gleichmäßig auf das logische Raster skaliert wird. |

## 8.2 Farbpalette

Es wird ausschließlich die folgende 16-Farben-Palette verwendet (PICO-8-Palette, frei nutzbar).
Abdunkeln (Overlays) per Dither-Muster oder Abbildung auf dunklere Palettenfarben.

| Idx | Hex | Name | Typische Verwendung |
|-----|-----|------|---------------------|
| 0 | `#000000` | Schwarz | Outlines, Augen, Ninja-Huhn |
| 1 | `#1D2B53` | Dunkelblau | Void-Tiefe, Overlay |
| 2 | `#7E2553` | Dunkelviolett | Schatten |
| 3 | `#008751` | Dunkelgrün | Gras-Schatten, Hügel |
| 4 | `#AB5236` | Braun | Erde, Holz, Brücke, Zaun |
| 5 | `#5F574F` | Dunkelgrau | Stein-Schatten, Ninja-Huhn |
| 6 | `#C2C3C7` | Hellgrau | Stein, ausgegraute Buttons |
| 7 | `#FFF1E8` | Weiß | Henni, Wolken, Text |
| 8 | `#FF004D` | Rot | Kamm, Ninja-Stirnband, Schrei-Bereich, Fehler |
| 9 | `#FFA300` | Orange | Schnabel, Füße, Coin-Schatten |
| 10 | `#FFEC27` | Gelb | Coins, Goldie, Heu, Highscore |
| 11 | `#00E436` | Grün | Gras, Reden-Bereich |
| 12 | `#29ADFF` | Hellblau | Himmel |
| 13 | `#83769C` | Lavendel | ferne Hügel/Berge |
| 14 | `#FF77A8` | Rosa | Wangen, Glitzer |
| 15 | `#FFCCAA` | Pfirsich | Highlights |

## 8.3 Szenerie

| ID | Element | Beschreibung |
|----|---------|--------------|
| FR-GR-10 | Himmel | Hellblau (12), im unteren Drittel in Bändern (gedithert) zu Dunkelblau (1) übergehend → "Void" wirkt bodenlos. |
| FR-GR-11 | Parallax (SOLL) | 2 Ebenen: Wolken (Faktor 0,2) und ferne Hügel in Lavendel (Faktor 0,4) relativ zur Kamera, nahtlos wiederholt. |
| FR-GR-12 | Inseln | Oben 1 Kachelreihe Gras (11 mit 3-Schattierung), darunter Erde (4) mit Steinchen; Unterseite verjüngt sich über 2–3 Kachelreihen zu einer Spitze. Kanten mit 1-px-Outline (0). |
| FR-GR-13 | Brückensegment | 8 × 4 Holzplanke (4/9), Seilandeutung oben; Zustände: intakt, wackelnd (±1 px Versatz), fallend (rotiert/zerbricht in 2 Teile optional). |
| FR-GR-14 | Hindernisse | Heuballen (10/9), Zaun (4), Steinblock (6/5), Turm (Heu + Kiste). Je 1 statischer Frame. |

## 8.4 Sprites & Animationen

Alle Sprites als palettenindizierte Arrays im Quellcode, mit einem Transparenzindex.
Die drei Hühner teilen sich dieselben Sprite-Formen; Unterschiede per **Paletten-Tausch** plus
Zusatzpixel (z. B. Stirnband) sind erlaubt.

### Huhn (16 × 16, Blickrichtung rechts)

| Animation | Frames | Frame-Dauer | Beschreibung |
|-----------|--------|-------------|--------------|
| Idle | 2 | 500 ms | Leichtes Wippen / Blinzeln |
| Walk | 4 | 80–160 ms (schneller bei höherer Geschwindigkeit) | Beine abwechselnd, Kopf nickt |
| Jump | 1 | – | Flügel halb hoch, Beine angezogen (Steigphase) |
| Flap | 2 | 60 ms | Flügel oben/unten, schnell |
| Fall | 1 | – | Flügel hoch, Augen groß |
| Push (KANN) | 1 | – | Gegen Hindernis gedrückt |

### Hühner-Varianten

| # | Name | Körper | Akzent | Preis |
|---|------|--------|--------|-------|
| 1 | **HENNI** | Weiß (7) mit Hellgrau-Schatten (6) | roter Kamm (8), Schnabel/Füße orange (9) | gratis (Standard) |
| 2 | **GOLDIE** | Gelb (10) mit Orange-Schatten (9) | roter Kamm, rosa Wangen (14), 1-px Glanzpunkt (7) | 50 Coins |
| 3 | **NINJA** | Dunkelgrau (5) mit Schwarz (0) | rotes Stirnband (8) mit flatterndem Bandende (2 Frames), Schnabel orange | 150 Coins |

### Weitere Sprites

| Sprite | Größe | Frames |
|--------|-------|--------|
| Coin | 8 × 8 | 4 (Rotation), 120 ms |
| Coin-Glitzer (SOLL) | 8 × 8 | 3, 60 ms, einmalig |
| Icons: Coin (klein), Schloss, Lautsprecher an/aus, Pause, Pfeil links/rechts | 7 × 7 bis 12 × 12 | 1 |
| Federn-Partikel beim Flattern (KANN) | 2 × 2 | – |
| Staub beim Landen (KANN) | 3 × 3 | 3 |

## 8.5 Darstellungsreihenfolge (von hinten nach vorne)

1. Himmel/Void-Verlauf
2. Parallax-Ebenen
3. Inseln, Brücken, Hindernisse
4. Coins
5. Huhn, Partikel
6. HUD
7. Overlays (Pause, Countdown, Dialoge)
