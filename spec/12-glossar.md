# 12 – Glossar

| Begriff | Bedeutung |
|---------|-----------|
| **Run** | Ein einzelner Spieldurchgang vom Countdown bis zum Tod oder Abbruch. |
| **Void** | Der bodenlose Abgrund unter den Inseln. Hineinfallen beendet den Run. |
| **Insel** | Schwebende, begehbare Plattform. |
| **Lücke** | Horizontaler Abstand zwischen zwei Inseln (Void dazwischen). |
| **Brücke / Brückensegment** | Brüchige Holzbrücke über einer Lücke, aus 8 px breiten Segmenten, die nach Standzeit einstürzen. |
| **Hindernis** | Festes Objekt auf einer Insel (Heuballen, Zaun, Stein, Turm), das seitlich blockiert und übersprungen werden muss. |
| **Coin** | Sammelobjekt; Währung zum Freischalten von Hühnern. |
| **Run-Coins** | In einem Run eingesammelte Coins (werden am Ende gutgeschrieben). |
| **Guthaben** | Gesamtzahl gespeicherter Coins. |
| **Score** | Weiteste erreichte Distanz eines Runs in Metern (1 m = 8 logische px). |
| **Highscore** | Bester Score aller Runs (persistent). |
| **STILL / TALK / SHOUT** | Die drei aus dem Mikrofonpegel abgeleiteten Eingabezustände (Stille / Reden / Schreien). |
| **intensity** | Wert 0–1 innerhalb des TALK-Bereichs, steuert die Laufgeschwindigkeit. |
| **Grundrauschen (Noise Floor)** | Bei der Kalibrierung gemessener Umgebungspegel in dBFS. |
| **dBFS** | Dezibel relativ zum digitalen Vollausschlag (0 dBFS = Maximum, negative Werte = leiser). |
| **Gating** | Ignorieren des Mikrofons während und kurz nach eigenen Sounds (Rückkopplungsschutz). |
| **Flattern** | Gehaltener Schrei in der Luft: langsameres Sinken, schnelleres Vorwärtskommen. |
| **Ausdauer (Stamina)** | Begrenzte Flatterzeit pro Luftphase, wird bei Landung aufgefüllt. |
| **Coyote-Time** | Kurze Zeit nach Verlassen einer Kante, in der ein Sprung noch erlaubt ist. |
| **Chunk** | Baustein der prozeduralen Map (z. B. Lücke, Brücke, Hindernis). |
| **Landezone** | Flacher, hindernisfreier Bereich nach einem Herausforderungs-Chunk. |
| **Stufe** | Schwierigkeitsstufe 1–3, abhängig von der Distanz. |
| **Logischer Pixel** | Pixel der 155 × 155 Spielauflösung; entspricht 3 × 3 physischen Pixeln. |
| **Safe-Kreis** | Bereich des runden Displays (Radius 72 logische px um die Mitte), in dem UI garantiert sichtbar ist. |
| **NVS** | Non-Volatile Storage – nichtflüchtiger Schlüssel-Wert-Speicher des ESP32. |
| **Tuning-Konstante (`K_*`)** | Zentral definierter Zahlenwert zum Feinabstimmen des Spiels. |
