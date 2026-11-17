# 03 – User Stories

Format: *Als &lt;Rolle&gt; möchte ich &lt;Ziel&gt;, damit &lt;Nutzen&gt;.*
Rolle ist immer **Spieler:in** (im Folgenden "ich"), außer anders angegeben.
Prio nach MoSCoW (M/S/C). Verweise zeigen auf Use Cases (UC) und detaillierte Anforderungen (FR).

---

## Epic A – Start & Hauptmenü

### US-01 – Spiel starten nach dem Einschalten (M)
Als Spieler:in möchte ich nach dem Einschalten schnell im Hauptmenü landen, damit ich sofort spielen kann.
- **AK1:** Nach dem Booten erscheint ein Splash-Screen mit Spieltitel und Huhn für 1–2 s.
- **AK2:** Spätestens 3 s nach dem Einschalten ist das Hauptmenü bedienbar.
- **AK3:** Ein Tippen oder Tastendruck während des Splashs springt sofort ins Hauptmenü.
- → UC-01, FR-UI-01

### US-02 – Coins und Highscore im Hauptmenü sehen (M)
Als Spieler:in möchte ich im Hauptmenü mein Coin-Guthaben und meinen persönlichen Highscore sehen, damit ich meinen Fortschritt kenne.
- **AK1:** Das Hauptmenü zeigt das Coin-Gesamtguthaben mit Coin-Symbol.
- **AK2:** Das Hauptmenü zeigt den Highscore in Metern (z. B. `HI 312 m`); bei 0 wird `HI 0 m` angezeigt.
- **AK3:** Beide Werte sind nach einem Neustart des Geräts unverändert.
- → UC-02, FR-UI-10ff., FR-PS-01ff.

### US-03 – Zwischen drei Hühnern blättern (M)
Als Spieler:in möchte ich zwischen drei Hühnern blättern, damit ich mir eines aussuchen kann.
- **AK1:** Links-/Rechts-Pfeile (Touch) und die linke/rechte Taste blättern zyklisch durch die drei Hühner.
- **AK2:** Für jedes Huhn werden Sprite (animiert), Name und Status (gewählt / verfügbar / Preis + Schloss) angezeigt.
- **AK3:** Beim Öffnen des Menüs ist das zuletzt gespielte Huhn vorausgewählt.
- → UC-03, FR-UI-12

### US-04 – Huhn kaufen (M)
Als Spieler:in möchte ich gesperrte Hühner mit Coins kaufen, damit ich einen Anreiz zum Sammeln habe.
- **AK1:** Bei einem gesperrten Huhn zeigt der Aktions-Button `KAUFEN <Preis>`.
- **AK2:** Reicht das Guthaben, erscheint ein Bestätigungsdialog (`Für <Preis> kaufen?` – `JA` / `NEIN`).
- **AK3:** Nach Bestätigung wird der Preis abgezogen, das Huhn dauerhaft freigeschaltet und direkt gewählt.
- **AK4:** Reicht das Guthaben nicht, ist der Button ausgegraut und zeigt `FEHLEN <Differenz>`; ein Antippen löst nur ein kurzes Fehler-Feedback aus, es wird nichts abgezogen.
- **AK5:** Ein Kauf ist auch nach Stromverlust direkt nach dem Kauf erhalten.
- → UC-04, FR-SH-01ff.

### US-05 – Spiel aus dem Menü starten (M)
Als Spieler:in möchte ich mit einer Aktion einen Run starten, damit ich schnell loslegen kann.
- **AK1:** Bei einem freigeschalteten Huhn zeigt der Aktions-Button `START`.
- **AK2:** Start ist per Touch auf `START` oder per **langem Druck** auf die rechte Taste möglich.
- **AK3:** Das angezeigte Huhn wird als gewähltes Huhn gespeichert und im Run verwendet.
- → UC-05

### US-06 – Ton ein-/ausschalten (S)
Als Spieler:in möchte ich die Sounds abschalten können, damit sie die Sprachsteuerung nicht stören oder in leiser Umgebung nicht nerven.
- **AK1:** Ein Lautsprecher-Symbol im Hauptmenü schaltet Ton an/aus (Touch) bzw. langer Druck auf die linke Taste.
- **AK2:** Die Einstellung bleibt nach Neustart erhalten.
- → UC-11, FR-AU-20

---

## Epic B – Sprachsteuerung

### US-07 – Mikrofon an die Umgebung anpassen (M)
Als Spieler:in möchte ich, dass sich das Spiel an die Umgebungslautstärke anpasst, damit die Steuerung im ruhigen Zimmer wie auch in lauter Umgebung funktioniert.
- **AK1:** Vor jedem Run läuft ein Countdown (`3 – 2 – 1 – LOS!`), während dem das Grundrauschen gemessen wird.
- **AK2:** Während der Messung wird der Hinweis `Psst... leise!` angezeigt und es wird kein Sound abgespielt.
- **AK3:** Die Schwellen für "Reden" und "Schreien" werden relativ zum gemessenen Grundrauschen gesetzt.
- → UC-05 (include UC-05a), FR-AU-01ff.

### US-08 – Durch Reden laufen (M)
Als Spieler:in möchte ich, dass das Huhn läuft, solange ich rede oder flüstere, damit ich es mit der Stimme steuern kann.
- **AK1:** Bei Pegel im Bereich "Reden" läuft das Huhn vorwärts.
- **AK2:** Je lauter innerhalb des Bereichs, desto schneller (stufenlos zwischen Min- und Max-Laufgeschwindigkeit).
- **AK3:** Bei Stille bleibt das Huhn innerhalb von ≤ 150 ms stehen.
- → UC-06, FR-GP-01ff.

### US-09 – Durch Schreien springen und flattern (M)
Als Spieler:in möchte ich, dass das Huhn springt, wenn ich schreie, und flattert, solange ich weiterschreie, damit ich Lücken und Hindernisse überwinden kann.
- **AK1:** Erreicht der Pegel den Bereich "Schreien", während das Huhn am Boden steht, springt es (Reaktionszeit ≤ 100 ms).
- **AK2:** Hält der Schrei in der Luft an, flattert das Huhn sichtbar (Flügel-Animation), sinkt langsamer und bewegt sich schneller vorwärts.
- **AK3:** Flattern ist pro Sprung zeitlich begrenzt (Flatter-Ausdauer); die Ausdauer füllt sich bei der Landung wieder auf.
- **AK4:** Ohne Flattern ist der Sprung eine feste Wurfparabel.
- → UC-06, FR-GP-10ff.

### US-10 – Lautstärke sehen (S)
Als Spieler:in möchte ich sehen, wie laut das Spiel mich gerade wahrnimmt, damit ich lerne, wie laut ich reden/schreien muss.
- **AK1:** Im HUD zeigt ein Pegelbalken den aktuellen Pegel mit zwei Markierungen (Reden-Schwelle, Schrei-Schwelle).
- **AK2:** Der Balken ist farblich in Still (grau) / Reden (grün) / Schreien (rot) unterteilt.
- → FR-UI-31

---

## Epic C – Welt & Hindernisse

### US-11 – Über Inseln laufen (M)
Als Spieler:in möchte ich über schwebende Inseln laufen und über Lücken springen, damit das Spiel eine Herausforderung bietet.
- **AK1:** Inseln sind begehbar; zwischen ihnen liegen Lücken ins Void.
- **AK2:** Läuft das Huhn über eine Kante oder verfehlt eine Insel, fällt es.
- → UC-06, FR-MP-01ff.

### US-12 – Brüchige Brücken (M)
Als Spieler:in möchte ich, dass Brücken einstürzen, wenn ich zu lange darauf stehe, damit ich unter Zeitdruck weiterlaufen muss.
- **AK1:** Brücken bestehen aus einzelnen Segmenten.
- **AK2:** Jedes Segment, auf dem das Huhn steht, beginnt nach kurzer Zeit zu wackeln und stürzt danach ab.
- **AK3:** Beim zügigen Überqueren in Laufgeschwindigkeit stürzt kein Segment unter dem Huhn ein.
- **AK4:** Eingestürzte Segmente kommen nicht zurück.
- → FR-MP-10ff.

### US-13 – Hindernisse überspringen (M)
Als Spieler:in möchte ich Hindernissen begegnen, die ich überspringen muss, damit ich das Schreien gezielt einsetzen muss.
- **AK1:** Hindernisse blockieren das Huhn seitlich (es läuft nicht hindurch).
- **AK2:** Auf Hindernissen kann man stehen.
- **AK3:** Das Berühren eines Hindernisses tötet nicht; das Huhn wird nur gestoppt.
- → FR-MP-20ff.

### US-14 – Ins Void fallen (M)
Als Spieler:in möchte ich, dass ein Fall ins Void den Run beendet, damit Fehler Konsequenzen haben.
- **AK1:** Fällt das Huhn unter den unteren Bildschirmrand, endet der Run mit Game Over.
- **AK2:** Vor dem Game-Over-Screen ist der Fall kurz sichtbar (Fall-Animation ≤ 1 s).
- → UC-09, FR-GP-30

### US-15 – Endlose, faire Map (M)
Als Spieler:in möchte ich, dass jede Runde anders aussieht, schwieriger wird und immer lösbar ist, damit es spannend und fair bleibt.
- **AK1:** Die Map wird pro Run zufällig aus Bausteinen generiert.
- **AK2:** Die Schwierigkeit steigt mit der Distanz in drei Stufen.
- **AK3:** Jede Lücke und jedes Hindernis ist mit den Bewegungsfähigkeiten des Huhns nachweislich überwindbar.
- → FR-MP-30ff.

---

## Epic D – Score, Coins, Game Over

### US-16 – Score sehen (M)
Als Spieler:in möchte ich während des Laufens meinen Score (Distanz) sehen, damit ich weiß, wie gut ich bin.
- **AK1:** Der Score in Metern wird oben im HUD angezeigt und steigt mit der weitesten erreichten Position.
- → FR-GP-40

### US-17 – Coins einsammeln (M)
Als Spieler:in möchte ich Coins einsammeln, damit ich Hühner kaufen kann.
- **AK1:** Berührt das Huhn einen Coin, verschwindet er und der Run-Coin-Zähler im HUD erhöht sich um 1.
- **AK2:** Beim Einsammeln ertönt ein kurzer Sound (wenn Ton an).
- **AK3:** Manche Coins sind nur durch Springen/Flattern erreichbar.
- → UC-07, FR-GP-50ff.

### US-18 – Ergebnis sehen (M)
Als Spieler:in möchte ich nach einem Run mein Ergebnis sehen, damit ich mich verbessern will.
- **AK1:** Der Game-Over-Screen zeigt Score, eingesammelte Coins des Runs und das neue Gesamtguthaben.
- **AK2:** Bei neuem Highscore wird `NEUER HIGHSCORE!` hervorgehoben angezeigt.
- **AK3:** Coins und Highscore sind zu diesem Zeitpunkt bereits gespeichert.
- → UC-09

### US-19 – Nochmal oder zurück ins Menü (M)
Als Spieler:in möchte ich nach dem Game Over direkt nochmal spielen oder ins Menü zurückkehren.
- **AK1:** Buttons `NOCHMAL` (rechte Taste) und `MENÜ` (linke Taste) sind vorhanden.
- **AK2:** In der ersten 1 s nach Erscheinen des Screens werden Eingaben ignoriert (Schutz vor Fehlklicks).
- → UC-10

### US-20 – Pausieren (M)
Als Spieler:in möchte ich das Spiel pausieren, damit ich unterbrechen kann.
- **AK1:** Pause per Touch auf das Pause-Symbol oder kurzer Druck auf die linke Taste.
- **AK2:** Im Pausenmenü: `WEITER` (rechte Taste / Touch) und `MENÜ` (linke Taste / Touch).
- **AK3:** Während der Pause wird das Mikrofon ignoriert; die Welt steht still (auch Brücken-Timer).
- **AK4:** `MENÜ` beendet den Run regulär: Coins werden gutgeschrieben, Score zählt für den Highscore.
- **AK5:** Nach `WEITER` folgt ein kurzer Countdown (1 s), bevor das Spiel weiterläuft.
- → UC-08

---

## Epic E – Feedback & Entwicklung

### US-21 – Kurze Sounds (S)
Als Spieler:in möchte ich kurze Sounds bei Start, Coin, Game Over und Kauf hören, damit sich das Spiel lebendig anfühlt.
- **AK1:** Sounds sind kurz (≤ 500 ms) und leise genug, um die Sprachsteuerung nicht auszulösen.
- **AK2:** Während ein Sound läuft, wird er nicht als Spracheingabe interpretiert.
- → FR-AU-10ff.

### US-22 – Vibration (S)
Als Spieler:in möchte ich bei Kauf und Game Over eine kurze Vibration spüren.
- → FR-AU-30

### US-23 – Entwickler: Steuerung ohne Stimme testen (S)
Als Entwickler:in möchte ich die Sprachsteuerung per Tasten simulieren und Pegelwerte seriell ausgeben können, damit ich das Spiel ohne Schreien testen und die Schwellen tunen kann.
- **AK1:** Mit Build-Flag `SIM_INPUT` gilt im Spiel: linke Taste gedrückt halten = "Reden", rechte Taste gedrückt halten = "Schreien"; Pause dann nur per Touch.
- **AK2:** Mit Build-Flag `DEBUG_LOG` werden 1×/s Pegel (dBFS), Grundrauschen, Schwellen, Eingabezustand und FPS seriell ausgegeben.
- → FR-DB-01ff.
