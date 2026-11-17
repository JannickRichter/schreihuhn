# 01 – Vision & Scope

## 1.1 Spielidee (Elevator Pitch)

Ein Huhn läuft durch eine seitlich scrollende Pixel-Art-Welt aus schwebenden Inseln über einem
endlosen Abgrund ("Void"). Gesteuert wird **ausschließlich mit der Stimme** über das eingebaute
Mikrofon:

| Stimme | Reaktion des Huhns |
|--------|--------------------|
| Stille | Huhn bleibt stehen |
| Flüstern / normales Reden | Huhn läuft vorwärts (je lauter, desto schneller) |
| Schreien | Huhn springt; hält man den Schrei, flattert es mit den Flügeln und gleitet weiter nach vorne |

Auf dem Weg gibt es Lücken, brüchige Brücken, die einstürzen, wenn man zu lange auf ihnen steht,
und Hindernisse, die übersprungen werden müssen. Wer danebenspringt, fällt ins Void → Game Over.
Je weiter man kommt, desto höher der **Score**. Unterwegs sammelt man **Coins**, mit denen man im
Hauptmenü zwei weitere Hühner freischalten kann. Der persönliche **Highscore** wird gespeichert.

## 1.2 Ziele des MVP

1. Spielbarer, unterhaltsamer Kern-Loop: *Menü → Run → Game Over → Menü/Nochmal*.
2. Zuverlässige, kalibrierte Sprachsteuerung in drei Stufen (Still / Reden / Schreien).
3. Endlose, prozedural generierte Map mit steigender Schwierigkeit und garantierter Lösbarkeit.
4. Meta-Progression: Coins sammeln, Hühner kaufen, Highscore – dauerhaft gespeichert.
5. Läuft **nativ** auf dem M5Stack StopWatch, flüssig (≥ 30 FPS), bedienbar per Touch und den
   zwei oberen Tasten.

## 1.3 Zielgruppe & Nutzungskontext

- Einzelspieler, Gelegenheitsspieler, Spaß-/Party-Charakter ("alle schreien das Gerät an").
- Gerät wird in der Hand gehalten oder liegt auf dem Tisch, Abstand Mund–Mikrofon ca. 10–50 cm.
- Runs dauern typischerweise 30 Sekunden bis 3 Minuten.

## 1.4 In Scope (MVP)

| Bereich | Inhalt |
|---------|--------|
| Screens | Splash, Hauptmenü (inkl. Huhn-Auswahl/Kauf), Kalibrierung/Countdown, Spiel (HUD), Pause, Game Over |
| Steuerung im Spiel | Mikrofon (primär); Pause über Touch oder linke Taste |
| Steuerung in Menüs | Touch-Flächen (primär) + linke/rechte Taste |
| Map-Elemente | Inseln, Lücken/Void, brüchige Brücken, Hindernisse, Coins |
| Map | Endlos, prozedural aus Chunk-Bausteinen, 3 Schwierigkeitsstufen nach Distanz |
| Progression | Score (Meter), Highscore, Coins (Gesamtguthaben), 3 Hühner (1 gratis, 2 kaufbar) |
| Audio | Wenige kurze synthetisierte Sounds, globaler Ton An/Aus, Rückkopplungsschutz |
| Haptik | Kurze Vibration bei Kauf und Game Over (SOLL) |
| Persistenz | Coins, Highscore, freigeschaltete Hühner, gewähltes Huhn, Ton-Einstellung |
| Debug | Serielles Logging, Eingabe-Simulation über Tasten (Build-Flag) |

## 1.5 Out of Scope (MVP)

- **Software-Architektur** (Modulschnitt, Klassen, Tasks, Patterns) – wird von der
  implementierenden Partei frei gewählt, unter Einhaltung von [10](10-nicht-funktionale-anforderungen.md).
- Mehrspieler, Online-Funktionen, WLAN/Bluetooth, Online-Highscores.
- Mehrere Highscore-Einträge / Namenseingabe (nur **ein** persönlicher Highscore).
- Einstellungsmenü (außer Ton An/Aus); manuelle Mikrofon-Empfindlichkeit.
- Hühner mit unterschiedlichen Spielwerten (alle Hühner sind **rein kosmetisch**).
- Gegner, Power-ups, Leben/Checkpoints, handgebaute Level, Weltwechsel/Themes.
- Musik/Hintergrundmusik, Sample-basierte Sounds (WAV/MP3).
- IMU-/Neigungssteuerung, RTC-Nutzung, Akku-Anzeige, Energiesparmodi über das Gerätestandardverhalten hinaus.
- OTA-Updates, Lokalisierung (UI ist nur Deutsch).

## 1.6 Priorisierung (MoSCoW)

| Prio | Inhalte |
|------|---------|
| **Must** | Sprachsteuerung 3-stufig inkl. Kalibrierung; Physik (Laufen, Springen, Flattern, Fallen); Inseln, Lücken, Brücken, Hindernisse, Coins; Score + Highscore; Hauptmenü mit Coins/Highscore/Start; Huhn-Auswahl + Kauf; Persistenz; Game Over; Pause; Touch + Tasten; ≥ 30 FPS |
| **Should** | Sounds inkl. Rückkopplungsschutz; Ton An/Aus; Mikrofon-Pegelanzeige im HUD; Vibration; Parallax-Hintergrund; Coyote-Time; Debug-Simulation |
| **Could** | Partikel (Federn beim Flattern, Staub beim Landen); Display-Dimmen im Menü nach Inaktivität; "Neuer Highscore"-Fanfare-Animation |
| **Won't (MVP)** | Alles aus 1.5 |
