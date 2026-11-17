# 14. Welten, Biome und abwechslungsreiche Hindernisse

> Erweiterungsspezifikation zu [05-gameplay-regeln.md](05-gameplay-regeln.md),
> [06-screens-und-bedienung.md](06-screens-und-bedienung.md) und [08-grafik-pixel-art.md](08-grafik-pixel-art.md).

---

## 14.1 Ziele

Das Spiel erhält zwei ineinandergreifende Erweiterungen zur langfristigen Motivation und spielerischen Abwechslung:

1. **Abwechslungsreiche Hindernisse & Technikwechsel (`FR-OB-*`):** Statt ausschließlich über Blöcke am Boden zu springen, wechseln sich unterschiedliche Hindernistypen und Chunk-Kombinationen ab, die zwischen ruhigem **Unterlaufen (`TALK`)**, gezieltem **Sprung (`SHOUT`)**, **Sprung + Flattern (gehaltenes `SHOUT`)** und **Stufen-Rhythmus** umschalten lassen.
2. **Dynamische Welten / Biome (`FR-BM-*`) & Decorator-Architektur (`FR-DC-*`):** Während eines Laufs wechselt die Spielwelt automatisch und mit **weichem Übergang** zwischen **6 regulären Biomen** (**Greenland**, **Taiga**, **Schneewelt**, **Lavawelt**, **Dschungel**, **Sand**) plus dem speziellen temporären **Süßigkeiten-Biom (`Candy`)**. Jedes Biom hat eigenständige Parallax-Silhouetten (u. a. kantige Eisberge im Schnee, hohe Dschungelbäume & Tempel im Dschungel, Sonne & Dünen im Sand), Insel-Texturen, Hintergrund-Deko (über eine modulare Decorator-Kette im `MapGenerator` platziert) und thematisierte Hindernisse.
3. **Spezial-Perk „Weißes Päckchen“ (`FR-PK-*`):** Ein seltenes weißes Päckchen-Pickup verleiht für einige Sekunden erhöhte Sprung- & mühelose Flugkraft (Steigen bereits durch leises `TALK` statt `SHOUT`), begrenzt nach oben durch eine unsichtbare Bildschirm-Deckenbarriere, und versetzt die Welt vorübergehend in das grell blinkende **Süßigkeiten-Biom**.

---

## 14.2 Erweiterte Hindernisse & Technikwechsel (`FR-OB-*`)

### 14.2.1 Hindernistypen

| ID | Typ | Größe (B×H px) | Platzierung | Erforderliche Technik | Ab Stage |
|----|-----|-----------------|-------------|-----------------------|----------|
| `FR-OB-01` | `HayBale` | `8×8` | Boden | Kleiner Sprung | Stage 1 |
| `FR-OB-02` | `Fence` | `8×16` | Boden | Mittlerer Sprung | Stage 2 (als Variante auch in Combos) |
| `FR-OB-03` | `Stone` | `16×16` | Boden | Mittlerer Sprung | Stage 2 |
| `FR-OB-04` | `Tower` | `8×24` | Boden | Hoher / früher Sprung | Stage 3 |
| `FR-OB-05` | `Overhang` | `24×12` | Schwebend, Unterkante `20 px` über Boden (`ground_y - 32`) | **Unterlaufen (`TALK`)**; Springen darunter stößt den Kopf an (`FR-GP-22`) | Stage 1 (geringes Gewicht) / Stage 2+ |
| `FR-OB-06` | `WideBarrier` | `24×16` | Boden | **Sprung + Flattern (`SHOUT` halten)** oder Zwischenlandung auf der Oberkante | Stage 2 |
| `FR-OB-07` | `Pillar` | `16×24` | Boden | Kräftiger Sprung + kurzes Flattern (`SHOUT` halten) | Stage 3 |

### 14.2.2 Neue Chunk-Typen & Technikwechsel-Patterns

Alle neuen Chunks halten die bestehenden Lösbarkeitsregeln (`FR-MP-35`, `FR-MP-36`, `K_LANDING_ZONE_W >= 24 px`, keine horizontalen Hindernis-Überlappungen) ein:

| Chunk | Aufbau | Spieler-Technik | Stage |
|-------|--------|-----------------|-------|
| `ObstUnder` | Insel mit `Overhang` (`24×12`, `20 px` Bodenfreiheit), `16 px` Anlauf, `>= 24 px` Auslauf; lockende Coins in Bodennähe darunter | **Nicht schreien** → ruhig mit `TALK` unterlaufen | 1, 2, 3 |
| `ObstWide` | Insel mit `WideBarrier` (`24×16`), `16 px` Anlauf, `>= 24 px` Auslauf; Coin-Bogen oben drüber | `SHOUT` halten für **Sprung + Flattern** oder Zwischenlandung | 2, 3 |
| `ObstStair` | Gestaffeltes Hindernis: `HayBale` (`8×8`), `16 px` Abstand, dahinter `Fence` (`8×16`) oder `Pillar` (`16×24`), `>= 24 px` Auslauf | **Stufen-Rhythmus** (Sprung auf/über Vorstufe, sofort zweiter Sprung) oder langes Flattern | 2, 3 |
| `ComboUO` | `Overhang` → `24 px` flache Laufzone → `Fence` (Stage 2) oder `GapS` (Stage 3) → `>= 24 px` Auslauf | **Sofortiger Technikwechsel:** erst ruhig `TALK` (drunter), direkt danach `SHOUT` (drüber) | 2, 3 |

### 14.2.3 Lösbarkeits- & Kollisionsgarantien

- `FR-OB-10`: Jedes schwebende `Overhang`-Hindernis MUSS mindestens `K_OVERHANG_CLEAR_PX = 20 px` Bodenfreiheit unterhalb aufweisen (`20 px > K_HITBOX_H = 13 px` mit `7 px` Sicherheitsreserve).
- `FR-OB-11`: Alle Bodenhindernisse (inkl. `WideBarrier` `16 px` und `Pillar` `24 px`) bleiben per `SolvabilityChecker::canClearObstacle` mit `Jump + Flap` sicher überwindbar.
- `FR-OB-12`: Hindernisse dürfen sich innerhalb eines Chunks und über Chunk-Grenzen hinweg nicht horizontal überlappen.

---

## 14.3 Dynamische Welten / Biome (`FR-BM-*`)

### 14.3.1 Progression & Wechsel-Logik

- `FR-BM-01`: Im Hauptmenü gibt es **keine** manuelle Weltauswahl. Jeder Lauf startet deterministisch in **Greenland** (`BiomeId::Greenland`).
- `FR-BM-02`: Nach Erreichen der nächsten Wechsel-Distanz (`K_BIOME_MIN_M = 45 m` bis `K_BIOME_MAX_M = 75 m`, deterministisch per Run-Seed gewürfelt) schaltet der Generator am `cursor_x` auf ein neues Biom um.
- `FR-BM-03`: Dasselbe Biom darf niemals zweimal direkt hintereinander gewählt werden (`next_biome != current_biome`).
- `FR-BM-04`: Jede erzeugte Insel und jedes Hindernis trägt seine bei der Generierung gültige `BiomeId`, sodass bestehende Inseln auf dem Bildschirm ihre Bodentextur nicht abrupt umschalten, sondern das neue Biom räumlich sauber mit der neuen Inselgrenze einzieht.

### 14.3.2 Die 6 regulären Welten & das Spezial-Biom

Alle Farben stammen strikt aus der 16-Farben-PICO-8-Palette (`spec/08` 8.2), mit deutlich unterscheidbaren Parallax-Silhouetten je Biom:

| Welt | Himmel & Atmosphäre | Parallax-Silhouetten | Inselboden & Void | Hintergrund-Deko (nicht kollidierend) | Partikel |
|------|---------------------|----------------------|-------------------|---------------------------------------|----------|
| **Greenland** (`Greenland`) | Hellblau (`C_LIGHT_BLUE`), weiße Wolken (`C_WHITE`, `C_LIGHT_GREY`) | Sanfte **gerundete grüne Hügel** (`C_GREEN`, `C_DARK_GREEN`) | Helle Grasdecke (`C_GREEN`/`C_DARK_GREEN`) auf brauner Erde (`C_BROWN`) | Kleine runde Laubbäume, Grasbüschel, kleine Blumen | keine |
| **Taiga** (`Taiga`) | Kühles Dunkelblau oben (`C_DARK_BLUE`) mit lavendelgrauem Horizont (`C_LAVENDER`), graue Wolken | **Asymmetrische schroffe Felsgrate** (`C_DARK_GREY`, `C_LIGHT_GREY`) vor dunklem Waldsaum (`C_DARK_GREEN`) | Dunkleres Moosgras (`C_DARK_GREEN`) auf steinig-graubrauner Erde (`C_DARK_GREY`/`C_BROWN`) mit Steinen | Fichten/Nadelbäume, weißstämmige Birken (`C_WHITE`/`C_BLACK` + `C_GREEN`/`C_YELLOW`), kleine Felsen | keine |
| **Schneewelt** (`Snow`) | Winterlich tiefes Blau (`C_DARK_BLUE`) mit kühl-grauem Dunst (`C_LIGHT_GREY`), graue Winterwolken | **Kantig-facettierte Eisberge & Gletscher-Tafeln** (asymmetrische Spitzen, Eisklippen in `C_LIGHT_BLUE`/`C_DARK_BLUE` & Schneefacetten in `C_WHITE`/`C_LIGHT_GREY`) | Dicke Schneedecke (`C_WHITE`, `C_LIGHT_BLUE`) auf kantig facettiertem Gletscher-/Felsboden (`C_DARK_GREY`/`C_DARK_BLUE`) | Verschneite Fichten (`C_DARK_GREEN` + `C_WHITE`), Eisfelsen | Sanft schräg fallende **Schneeflocken** (`1×1`/`2×1`, `C_WHITE`/`C_LIGHT_GREY`, Pool `K_AMBIENT_SNOW_COUNT = 20`) |
| **Lavawelt** (`Lava`) | Rauchiges Schwarz/Dunkelgrau/Dunkelviolett (`C_BLACK`/`C_DARK_GREY`/`C_DARK_PURPLE`) mit orange-rotem Horizont (`C_RED`/`C_ORANGE`) | **Flach abgestumpfte Vulkankegel** (`C_DARK_GREY`/`C_BLACK`) mit glühenden Kratern (`C_RED`/`C_ORANGE`/`C_YELLOW`) & Rauchwolken | Dunkelgrau-schwarzes Vulkangestein mit glühenden Lava-Adern; **animierter Lavafluss** am unteren Bildschirmrand | Verkohlte Basaltspitzen, glühende Lavasteine | Im Hintergrund schräg fallende **Kometen** (`C_YELLOW`/`C_ORANGE`/`C_RED`) & feine Glut-Funken |
| **Dschungel** (`Jungle`, `"DSCHUNGEL"`) | Tiefes Dschungelgrün/-blau (`C_DARK_BLUE`/`C_DARK_GREEN`), feuchter **Dunst-Nebel** im Horizont (`C_LIGHT_GREY`/`C_GREEN`) & diagonale **Sonnenstrahlen** (`C_YELLOW`/`C_WHITE`) durch die Kronen | **Hohe Urwald-Baumriesen** (lange Stämme + mehrstufiges Blätterdach `C_DARK_GREEN`/`C_BLACK`) & gestufte **Dschungeltempel** (`C_DARK_GREY`/`C_LIGHT_GREY` + Moos) | Üppige dunkelgrüne/grüne Dschungeldecke mit hängenden Ranken auf feuchtem Erdboden (`C_BROWN`/`C_DARK_GREY`) | Hohe Dschungelbäume, kleine **Dschungeltempel-Ruinen**, Farne | **Sehr leichter, sporadischer Dschungelregen** (`1×2` Tropfen in `C_LIGHT_BLUE`/`C_WHITE`, Pool `K_AMBIENT_RAIN_COUNT = 12`, deutlich dezenter als Schnee) |
| **Sand / Wüste** (`Sand`, `"WÜSTE"`) | Warmer Himmel (`C_LIGHT_BLUE` → `C_PEACH`/`C_ORANGE`) mit **glühender Sonne** oben rechts (`C_YELLOW`/`C_WHITE`) | **Weich geschwungene Sanddünen** mit Licht-/Schattenseite (`C_YELLOW`, `C_PEACH`, `C_ORANGE`) | Goldgelbe Sanddecke (`C_YELLOW`/`C_PEACH`) auf warmem Sandstein (`C_ORANGE`/`C_BROWN`) | **Zahlreiche Kakteen** (kleine Kugel- & hohe Kandelaber-Kakteen), Wüstensteine | keine (sonnige Hitze) |
| **Süßigkeiten** (`Candy`, `"ZUCKERRAUSCH!"`, *nur bei Perk*) | **Grell blinkende/pulsierende Farben** (Streifen-/Dither-Wellen in **Weiß, Violett/Lila, Pink, Hellblau, Gelb**) | **Riesen-Lutscher**, gestreifte **Zuckerstangen** & Pastell-Hügel (`C_PINK`/`C_LAVENDER`/`C_WHITE`) | Zuckerguss-Decke (`C_WHITE`/`C_PINK`) mit bunten Streuseln auf Waffelboden (`C_PEACH`/`C_LAVENDER`) | Lutscher, Bonbons (Konturen in `C_DARK_PURPLE`/`C_BLACK` für klare Lesbarkeit) | Funkelnde Candy-Sterne & Konfetti (`C_WHITE`, `C_PINK`, `C_YELLOW`, `C_LAVENDER`) |

> Hinweis: `K_REG_BIOME_COUNT = 6` umfasst `Greenland`, `Taiga`, `Snow`, `Lava`, `Jungle`, `Sand`. Das `Candy`-Biom wird ausschließlich temporär durch Aufnahme des Perk-Items aktiviert (`FR-PK-*`).

### 14.3.3 Biome-spezifische Hindernisse

`Sprites::drawObstacle(..., BiomeId biome)` passt Darstellung und Farben jedes Hindernistyps an alle Biome an (`Greenland`, `Taiga`, `Snow`, `Lava`, `Jungle` mit Moos/Tempelstein/Bambus, `Sand` mit Sandstein/Kaktus-Holz/Canyon-Säulen, `Candy` in grell-süßen Weiß/Lila/Pink-Tönen mit dunkler Kontur), bei unveränderter Pixel-Hitbox.

### 14.3.4 Weicher Biom-Übergang (`FR-BM-20..22`)

- `FR-BM-20`: Sobald der Spieler (`player.world_x`) die Weltgrenze `transition_x` erreicht (oder ein Perk aktiviert/beendet wird), startet eine weiche Atmosphären-Überblendung (`K_BIOME_TRANSITION_S = 1.4 s` bei regulären Wechseln, `0.45 s` beim Perk-Eintritt).
- `FR-BM-21`: Da die PICO-8-Palette diskrete 16 Farben nutzt, verwendet der `WorldRenderer` für Himmel, Horizont, Parallax-Silhouetten und Boden-Lava/Void eine **4×4-Bayer-Dither-Überblendung** zwischen altem und neuem Biom.
- `FR-BM-22`: Ein dezenter Biom-Schriftzug (`GREENLAND`, `TAIGA`, `SCHNEEWELT`, `LAVAWELT`, `DSCHUNGEL`, `WÜSTE`, `ZUCKERRAUSCH!`) erscheint für `K_BIOME_BANNER_S = 1.5 s` mittig unter dem Score.

---

## 14.4 Decorator-Pattern im Weltgenerator (`FR-DC-*`)

Zur sauberen, erweiterbaren Trennung zwischen **Grundgeometrie eines Chunks** (Inseln, Lücken, Brücken, Hindernisse, Coins) und **thematischer/spielmechanischer Ausschmückung** nutzt `MapGenerator` ein **Decorator Pattern** ohne dynamische Heap-Allokation (`NFR-06`):

- `FR-DC-01`: Das Interface `IChunkDecorator` definiert `decorate(const ChunkContext& ctx, WorldState& world, core::XorShift32& rng)`.
- `FR-DC-02`: `MapGenerator` erzeugt zunächst den Basis-Chunk (`ChunkContext`: `start_x`, `end_x`, `biome`, Liste der erzeugten Inseln/Ausschlusszonen um Hindernisse und Ziel-Landezone) und führt anschließend eine geordnete Kette statisch gehaltener Dekoratoren aus:
  1. `BiomePropDecorator`: Platziert je nach `ctx.biome` biom-spezifische Hintergrund-Requisiten (`DecorItem`) mit unterschiedlichen Dichten (z. B. hohe Kakteen-Dichte in `Sand`, hohe Bäume & Tempelruinen in `Jungle`, verschneite Fichten in `Snow`, etc.) unter Einhaltung der Hindernis-Ausschlusszonen.
  2. `PerkSpawnDecorator`: Platziert nach Ablauf der Abklingzeit (`K_PERK_COOLDOWN_M`) und bei ausreichender sicherer Insellänge ein `PerkItem` (weißes Päckchen) oberhalb einer begehbaren Fläche.
- `FR-DC-03`: Alle Dekoratoren arbeiten rein deterministisch über den übergebenen Run-PRNG (`AT-13` bleibt 100 % erfüllt) und schreiben nur in fest dimensionierte Ringpuffer (`world.decors`, `world.perks`).

---

## 14.5 Spezial-Perk „Weißes Päckchen“ & Flug-Modus (`FR-PK-*`)

- `FR-PK-01` **Item-Optik**: Das Perk-Pickup (`PerkItem`, `8×8 px`) wird als kleines **weißes, gefaltetes Briefchen/Päckchen** (`C_WHITE` mit `C_LIGHT_GREY`-Falzkanten, `C_BLACK`-Outline und leichtem Funkeln) gerendert und schwebt leicht wippend oberhalb von Inseln.
- `FR-PK-02` **Spawn-Regeln**: Platziert durch `PerkSpawnDecorator` ab ca. `28 m` nach Start mit einer Abklingzeit von mindestens `K_PERK_COOLDOWN_M = 60 m` (Wahrscheinlichkeit `~40 %` pro geeigneter flacher Insel/Landezone nach Ablauf des Cooldowns), niemals innerhalb von Hindernissen oder über Lücken.
- `FR-PK-03` **Dauer & Rückmeldung**: Nach Einsammeln (`SoundId::Perk`) läuft der Perk für `K_PERK_DURATION_S = 6.5 s`. Im HUD erscheint währenddessen eine blinkende Restzeit-Anzeige (`K_PERK_WARN_S = 1.2 s` Warnblinken vor Ende) sowie das Banner `ZUCKERRAUSCH!`.
- `FR-PK-04` **Höherer Sprung & müheloser Flug (`TALK` reicht!)**:
  - Während des Perks entfällt das Stamina-Limit; bereits **leichtes Reden (`VoiceState::Talk` oder `Shout`)** erzeugt Auftrieb nach oben (proportional zur Sprachintensität zwischen `K_PERK_FLOAT_VY` und `K_PERK_CLIMB_VY`) sowie erhöhte Vorwärtsgeschwindigkeit (`K_PERK_FLY_VX = 90 px/s`).
  - Bei Stille (`VoiceState::Still`) sinkt das Huhn nur sehr sanft (`K_PERK_GRAVITY = 180 px/s²`, max. Fallgeschwindigkeit `K_PERK_MAX_FALL = 50 px/s`).
  - Beim Absprung vom Boden wirkt eine stärkere Sprungkraft (`K_PERK_JUMP_VY = 240 px/s`).
- `FR-PK-05` **Obere Bildschirm-Barriere**: Während des Flugs verhindert eine feste obere Deckenbarriere bei `K_PERK_CEILING_Y = 18.0f px` (unterhalb des HUD-Scores und komplett innerhalb des runden Safe-Circles), dass das Huhn oben aus dem Bildschirm herausfliegen kann (`y` auf `>= K_PERK_CEILING_Y` geklemmt, `vy = std::max(0.0f, vy)`).
- `FR-PK-06` **Temporäres Süßigkeiten-Biom**: Mit Aufnahme des Päckchens blendet die Atmosphäre in `BiomeId::Candy` über (blinkend-pulsierender Himmel in Weiß/Lila/Pink/Gelb/Hellblau, Candy-Berge/-Lutscher/-Zuckerstangen, bunte Partikel). Läuft der Timer ab (oder endet der Run), blendet die Welt wieder weich auf das darunterliegende Welt-Biom zurück.

### 14.3.5 Technische Rahmenbedingungen

- `FR-BM-30`: Alle Deko-Objekte (`DecorItem`) und Umgebungs-Partikel (`AmbientParticle`) liegen in fest dimensionierten Ring-/Array-Puffern ohne dynamische Heap-Allokationen im Frame-Loop (`NFR-06`).
- `FR-BM-31`: Umgebungs-Partikel sind rein dekorativ, beeinflussen weder Physik noch Kollision und werden so gedrosselt gezeichnet, dass Huhn, Coins, Brücken und Hindernisse jederzeit eindeutig erkennbar bleiben.

---

## 14.6 Touch-Hitboxen Pause & Game Over (`FR-UI-P3` Ergänzung)

Analog zu den vergrößerten Safe-Circle-Touch-Zielen im Hauptmenü erhalten auch die Buttons im Pause-Screen (`MENÜ`, `WEITER`) und Game-Over-Screen (`MENÜ`, `NOCHMAL`) vergrößerte Finger-Hitboxen links und rechts (`UILayout::kPauseBtn*` / `UILayout::kGameOverBtn*`), während die sichtbaren Button-Rahmen sauber innerhalb des runden Safe-Circles (`M=(77,77), R=72`) gezeichnet bleiben.

---

## 14.7 Backlog: Unterwasserwelt (spätere Iteration)

> **Status:** Geplant für eine zukünftige Erweiterung (noch nicht in diesem Release aktiviert).

- **Biom `Underwater` (Unterwasserwelt):**
  - **Atmosphäre:** Tiefblau-türkiser Wassergrund (`C_DARK_BLUE`, `C_LIGHT_BLUE`), Lichtstrahlen von der Oberfläche, sanfter Seeboden im Parallax.
  - **Deko & Partikel:** Wiehende Seegras-Halme, bunte Korallen (`C_PINK`, `C_ORANGE`, `C_LAVENDER`) und stetig **aufsteigende Luftblasen-Partikel** (`C_WHITE`/`C_LIGHT_BLUE`).
  - **Hindernisse & Optionale Mechanik:** Versunkene Ruinen-Bögen, Seeigel/Korallenblöcke; optional modifizierte Unterwasser-Schwerkraft (`0.65 × K_GRAVITY`, sanfteres Sinken & höherer Schwimm-Auftrieb bei `SHOUT`) mit angepasster `SolvabilityChecker`-Prüfung.
