# 10 – Nicht-funktionale Anforderungen

| ID | Kategorie | Anforderung | Messung / Nachweis |
|----|-----------|-------------|--------------------|
| NFR-01 | Performance | Im Spiel **≥ 30 FPS** im Mittel, kein Frame länger als 50 ms im 95. Perzentil (Ziel: 60 FPS, falls ohne Aufwand erreichbar). | `DEBUG_LOG`-FPS-Ausgabe über 2 min Spiel |
| NFR-02 | Performance | Physik läuft mit festem Zeitschritt (`K_PHYS_DT`), d. h. Spielverhalten ist unabhängig von der tatsächlichen Framerate. | Unit-Test: gleiche Eingabesequenz bei 30 und 60 Render-FPS → gleiche Position (± 1 px) |
| NFR-03 | Latenz | Schrei → Sprung ≤ 100 ms; Stille → Stillstand ≤ 150 ms (FR-AU-07). Touch/Taste → visuelle Reaktion ≤ 100 ms. | Logik-Test mit synthetischen Pegelsequenzen; Sichtprüfung |
| NFR-04 | Startzeit | Einschalten → Hauptmenü bedienbar ≤ 3 s. | Stoppuhr / Log-Zeitstempel |
| NFR-05 | Stabilität | 30 min Dauerbetrieb (Runs, Menüwechsel, Käufe) ohne Absturz, Watchdog-Reset oder Speicherleck (freier Heap nach 30 min ≥ 90 % des Werts nach 1 min). | `DEBUG_LOG` gibt zusätzlich freien Heap aus |
| NFR-06 | Speicher | Speicherverbrauch des Map-Generators ist beschränkt (Ringpuffer o. Ä.), unabhängig von der Run-Länge. | Unit-Test: 100 km Simulation, Elementanzahl beschränkt |
| NFR-07 | Robustheit | Fehlender/korrupter Speicherstand, nicht initialisierbares Mikrofon oder Audio führen nie zu Absturz oder Boot-Schleife. | Tests FR-PS-02, UC-01 1a |
| NFR-08 | Testbarkeit | Spiellogik (Physik, Kollision, Generator inkl. Lösbarkeit, Brücken, Score, Shop, Persistenz-Validierung, Mikrofon-Klassifizierung) ist ohne Hardware auf dem Host per Unit-Test prüfbar (RB-04). | `pio test -e native` grün |
| NFR-09 | Wartbarkeit | Alle `K_*`-Konstanten an einem zentralen Ort; keine magischen Zahlen für Tuning-Werte im restlichen Code. | Code-Review |
| NFR-10 | Wartbarkeit | Build ohne Warnungen mit `-Wall` für den eigenen Code (Warnungen aus Drittbibliotheken ausgenommen). | Build-Log |
| NFR-11 | Usability | Ein:e Erstspieler:in versteht die Steuerung ohne Anleitung aus Splash-Hinweis + Pegelbalken. | Plausibilitätsprüfung |
| NFR-12 | Usability | Alle Texte lesbar (mind. 7 logische px Schrifthöhe = ca. 2 mm), Kontrast durch Outline gesichert. | Sichtprüfung |
| NFR-13 | Energie | Keine aktive Funkschnittstelle; Display-Helligkeit mit fester, moderater Stufe (`K_BRIGHTNESS`, Startwert 60 %). | Code-Review |
| NFR-14 | Lizenz | Keine Assets/Code mit unklarer Lizenz; alle Grafiken/Sounds eigenerstellt im Code. Drittbibliotheken nur mit permissiver Lizenz (MIT/BSD/Apache o. ä.). | Code-Review |
| NFR-15 | Fairness | Keine unlösbare Stelle in der Map (FR-MP-36). | Fuzz-Test über 1000 Seeds × 3000 m |
