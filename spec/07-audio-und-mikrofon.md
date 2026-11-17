# 07 – Audio & Mikrofon

## 7.1 Mikrofon-Signalverarbeitung

### Tuning-Konstanten (Startwerte)

| Konstante | Wert | Beschreibung |
|-----------|------|--------------|
| `K_MIC_RATE` | 16 000 Hz | Abtastrate (andere Rate zulässig, wenn Hardware/Lib es erfordert) |
| `K_MIC_FRAME_MS` | 20 ms | Analyse-Fensterlänge |
| `K_DC_FILTER` | aktiv | DC-Offset-Entfernung (Hochpass ~ 80–100 Hz oder Mittelwertabzug) vor der Pegelberechnung |
| `K_SMOOTH_ATTACK` | 0,6 | Glättungsfaktor (EMA), wenn Pegel steigt |
| `K_SMOOTH_RELEASE` | 0,25 | Glättungsfaktor (EMA), wenn Pegel fällt |
| `K_CALIB_MS` | 1500 ms | Dauer der Grundrausch-Messung |
| `K_FLOOR_MIN_DB` | −70 dBFS | Untergrenze Grundrauschen |
| `K_FLOOR_MAX_DB` | −40 dBFS | Obergrenze Grundrauschen |
| `K_TALK_OFFSET_DB` | +12 dB | Reden-Schwelle über Grundrauschen |
| `K_SHOUT_OFFSET_DB` | +28 dB | Schrei-Schwelle über Grundrauschen |
| `K_SHOUT_ABS_MIN_DB` | −30 dBFS | Schrei-Schwelle ist mindestens so hoch (verhindert Schreien-Erkennung bei normaler Sprache in sehr leiser Umgebung) |
| `K_HYSTERESIS_DB` | 3 dB | Hysterese beim Verlassen eines Zustands |
| `K_SHOUT_MIN_FRAMES` | 2 | So viele aufeinanderfolgende Frames über Schrei-Schwelle, bevor `SHOUT` aktiv wird |
| `K_MIC_GAIN` | Hardware-Default | Verstärkung; vom Implementierer so einzustellen, dass normale Sprache in 30 cm nicht clippt und Schreien nicht dauerhaft clippt |

> Die dB-Werte sind Startwerte. Der Implementierer MUSS sie in einem einzigen Konfigurationsort
> halten. Endgültiges Tuning erfolgt auf echter Hardware mithilfe von `DEBUG_LOG` (UC-12).

### Anforderungen

| ID | Anforderung |
|----|-------------|
| FR-AU-01 | Pro Frame wird der RMS-Pegel berechnet und in dBFS umgerechnet: `L = 20·log10(rms / fullscale)`, untere Schranke −90 dBFS. |
| FR-AU-02 | Der Pegel wird asymmetrisch geglättet (`K_SMOOTH_ATTACK` beim Anstieg, `K_SMOOTH_RELEASE` beim Abfall). |
| FR-AU-03 | **Kalibrierung (UC-05a):** Über `K_CALIB_MS` werden die ungeglätteten Frame-Pegel gesammelt; Grundrauschen = **Median** der Werte, begrenzt auf `[K_FLOOR_MIN_DB, K_FLOOR_MAX_DB]`. |
| FR-AU-04 | Schwellen: `talk = floor + K_TALK_OFFSET_DB`; `shout = max(floor + K_SHOUT_OFFSET_DB, K_SHOUT_ABS_MIN_DB)`. Es MUSS gelten `shout ≥ talk + 10 dB` (sonst `shout` entsprechend anheben). |
| FR-AU-05 | **Klassifizierung mit Hysterese:** `STILL → TALK` wenn L ≥ talk; `TALK → STILL` wenn L < talk − Hysterese; `→ SHOUT` wenn L ≥ shout für `K_SHOUT_MIN_FRAMES` Frames; `SHOUT → TALK/STILL` wenn L < shout − Hysterese. |
| FR-AU-06 | `intensity` (für TALK) = `clamp((L − talk) / (shout − talk), 0, 1)`. |
| FR-AU-07 | **Latenz:** Zeit von Beginn eines Schreis bis Sprungbeginn ≤ 100 ms; vom Verstummen bis "Huhn steht" ≤ 150 ms (inkl. Bremsweg). |
| FR-AU-08 | Die Mikrofon-Verarbeitung darf die Framerate nicht unter die Vorgabe (NFR-01) drücken und keine Frames "verschlucken" (Audio-Puffer kontinuierlich auslesen, kein Überlauf). |
| FR-AU-09 | Das Mikrofon ist nur in Countdown und Spiel aktiv; in Menüs, Pause und Game Over wird es nicht ausgewertet (darf deaktiviert werden). |

## 7.2 Rückkopplungsschutz (Sound ↔ Mikrofon)

Problem: Spielsounds aus dem Lautsprecher dürfen nicht als Spracheingabe erkannt werden.

| ID | Anforderung |
|----|-------------|
| FR-AU-10 | **Gating:** Ab Start eines Sounds bis `K_GATE_TAIL_MS` = **150 ms** nach dessen Ende werden Mikrofon-Frames **verworfen**. Der zuletzt gültige geglättete Pegel und Eingabezustand werden **gehalten** – mit einer Ausnahme: Ein Übergang nach `SHOUT` kann während des Gatings nicht neu entstehen. |
| FR-AU-11 | Lässt die Hardware/Bibliothek kein gleichzeitiges Aufnehmen und Abspielen zu, darf das Mikrofon für die Dauer des Sounds gestoppt und anschließend neu gestartet werden. Die Gesamtunterbrechung (Sound + Neustart + Tail) DARF 400 ms nicht überschreiten; FR-AU-10 gilt sinngemäß. |
| FR-AU-12 | Im Spiel abgespielte Sounds MÜSSEN ≤ 150 ms lang sein (Coin-Sound). Längere Sounds (Start-Jingle, Game Over) werden nur gespielt, wenn das Mikrofon nicht ausgewertet wird (vor dem Countdown bzw. nach dem Tod). |
| FR-AU-13 | Die Vibration wird gleich behandelt wie ein Sound (Gating), falls sie während des Spiels genutzt wird. Im MVP vibriert es im Spiel nur nach dem Tod → kein Gating nötig. |
| FR-AU-14 | Standard-Lautstärke `K_VOLUME` = 40 % des Maximums (Tuning-Konstante). |

## 7.3 Sound-Katalog

Alle Sounds werden **synthetisch** erzeugt (Rechteck-/Dreieckston-Folgen über die Tonfunktion der
Audio-Bibliothek), keine Sample-Dateien.

| ID | Anlass | Beschreibung (Startwert) | Dauer | Wann |
|----|--------|--------------------------|-------|------|
| SND-START | Splash & Run-Start | 3 aufsteigende Töne C5–E5–G5 je 100 ms + G5 150 ms | ≈ 450 ms | vor dem Countdown |
| SND-CLICK | Menü-Navigation | 1 Ton 1500 Hz, 15 ms | 15 ms | Menüs |
| SND-COIN | Coin eingesammelt | B5 40 ms → E6 70 ms | 110 ms | im Spiel |
| SND-BUY | Kauf erfolgreich | C5–E5–G5–C6 je 60 ms | 240 ms | Menü |
| SND-ERROR | Kauf nicht möglich | 200 Hz, 120 ms | 120 ms | Menü |
| SND-GAMEOVER | Tod | G4–E4–C4 je 120 ms + C3 200 ms | ≈ 560 ms | nach dem Tod |
| SND-HISCORE | Neuer Highscore | kurze Fanfare C5–C5–G5 (je 80 ms) | 240 ms | nach SND-GAMEOVER auf dem Game-Over-Screen |
| SND-TOGGLE | Ton eingeschaltet | 1000 Hz 40 ms | 40 ms | Menü |

- Es gibt **keinen** Sprung-/Flatter-Sound (würde mit dem Schreien kollidieren und das Gating ständig auslösen).

## 7.4 Ton-Einstellung & Haptik

| ID | Anforderung |
|----|-------------|
| FR-AU-20 | Ist Ton aus, wird kein Sound abgespielt (auch kein Klick); Gating entfällt dann. |
| FR-AU-21 | Die Ton-Einstellung wird persistent gespeichert (siehe [09](09-persistenz-und-shop.md)). |
| FR-AU-30 | (SOLL) Vibration 120 ms bei erfolgreichem Kauf, 200 ms bei Game Over. Vibration ist unabhängig von der Ton-Einstellung aktiv. |
