# 02 – Hardware & Rahmenbedingungen

## 2.1 Zielgerät: M5Stack StopWatch (SKU C152)

| Komponente | Details | Relevanz für das Spiel |
|------------|---------|------------------------|
| SoC | ESP32-S3R8, Dual-Core LX7 @ 240 MHz | Spiellogik, Rendering, Audio |
| Speicher | 16 MB Flash, 8 MB PSRAM (OPI) | Framebuffer, Sprites, NVS-Persistenz |
| Display | 1,75" **rundes** AMOLED, **466 × 466 px**, Treiber CO5300 (QSPI) | Rundes Sichtfeld → Safe-Area beachten |
| Touch | CST820B, kapazitiv | Menüs, Pause |
| Tasten | 2 programmierbare Tasten **oben links / oben rechts**, 1 Power-Taste | Menü-Navigation; Power-Taste wird **nicht** belegt |
| Audio | ES8311 Codec, MEMS-Mikrofon, AW8737A Verstärker, 1 W Lautsprecher | Sprachsteuerung, Sounds |
| Haptik | Vibrationsmotor | Feedback (SOLL) |
| Sonstiges | BMI270 IMU, RX8130CE RTC, M5PM1 PMIC, 450 mAh Akku | Nicht genutzt (außer PMIC-Init, falls nötig) |

Offizielle Referenzen (bei Unklarheiten zu Pins/Treibern **immer** dort nachsehen):

- Produktdoku: https://docs.m5stack.com/en/core/StopWatch
- Arduino-Quickstart: https://docs.m5stack.com/en/arduino/stopwatch/program
- Factory-Firmware (ESP-IDF, Referenz für Peripherie-Init): https://github.com/m5stack/M5StopWatch-UserDemo

## 2.2 Technische Rahmenbedingungen (verbindlich)

| ID | Rahmenbedingung |
|----|-----------------|
| RB-01 | Das Spiel MUSS als **native Firmware** auf dem ESP32-S3 laufen (kein Webinterface, kein Streaming, kein Host-PC zur Laufzeit). |
| RB-02 | Build-System: **PlatformIO** (empfohlen, mit Arduino-Framework und M5Unified/M5GFX). Ein anderes Framework (z. B. ESP-IDF) ist zulässig, wenn es begründet im Entscheidungslog steht. |
| RB-03 | Das Projekt MUSS mit einem einzigen Befehl reproduzierbar bauen (z. B. `pio run -e stopwatch`). Alle Abhängigkeiten MÜSSEN mit fester Version/Commit gepinnt sein. |
| RB-04 | Die Spiellogik (Physik, Kollision, Map-Generator, Score, Shop, Mikrofon-Klassifizierung) MUSS **ohne Hardware auf dem Host-PC testbar** sein (z. B. PlatformIO `native`-Environment mit Unity-Tests). |
| RB-05 | Alle Grafiken und Sounds MÜSSEN im Quellcode erzeugt bzw. eingebettet werden (Sprite-Arrays, Tonfolgen). Es werden **keine externen Asset-Tools oder Downloads** zur Build-Zeit benötigt. |
| RB-06 | Keine Netzwerkfunktionen (WLAN/BT bleiben aus). |
| RB-07 | Code-Sprache (Bezeichner, Kommentare): Englisch. UI-Texte: Deutsch. |

### Empfohlene PlatformIO-Basis (laut M5Stack-Vorlage, vom Implementierer zu verifizieren)

```ini
[env:stopwatch]
platform = espressif32 @ 6.12.0
board = esp32s3box
framework = arduino
board_build.partitions = default_16MB.csv
board_upload.flash_size = 16MB
board_upload.maximum_size = 16777216
board_build.arduino.memory_type = qio_opi
monitor_speed = 115200
build_flags =
    -DESP32S3
    -DBOARD_HAS_PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1
lib_deps =
    ; Versionen/Commits pinnen!
    https://github.com/m5stack/M5Unified
    https://github.com/m5stack/M5GFX
    https://github.com/m5stack/M5PM1
    https://github.com/m5stack/M5IOE1
```

## 2.3 Bekannte technische Risiken (vom Implementierer zu behandeln)

| Risiko | Erwartete Behandlung |
|--------|----------------------|
| Mikrofon und Lautsprecher teilen sich ggf. den Codec/I2S-Bus; gleichzeitiges Aufnehmen und Abspielen evtl. nicht möglich. | Zulässig: Mikrofon für die Dauer eines Sounds stoppen und danach neu starten. Eingabezustand wird währenddessen gehalten (siehe [07](07-audio-und-mikrofon.md), FR-AU-10). |
| Zuordnung der Tasten (`BtnA`/`BtnB` o. Ä.) zu physisch links/rechts ist unklar. | Zuordnung so wählen, dass "links" = die Taste links oben, wenn das Display zum Nutzer zeigt. Zuordnung zentral als Konstante. |
| Vollbild-Rendering 466×466 @ 16 bit ist bandbreitenintensiv. | Rendern in niedriger logischer Auflösung und Hochskalieren (siehe [08](08-grafik-pixel-art.md)); Double-Buffering in PSRAM. |
| Touch-Koordinaten evtl. gespiegelt/rotiert. | Beim Start einmal korrekt konfigurieren; Mapping zentral als Konstante. |
