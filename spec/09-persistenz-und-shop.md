# 09 – Persistenz & Shop

## 9.1 Gespeicherte Daten

| Feld | Typ | Default | Bereich |
|------|-----|---------|---------|
| `schema_version` | uint8 | 1 | – |
| `coins` | uint32 | 0 | 0 … `K_COINS_MAX` = 999 999 |
| `highscore` | uint32 | 0 | 0 … 999 999 (Meter) |
| `unlocked` | uint8 Bitmaske | `0b001` | Bit 0 = HENNI (immer gesetzt), Bit 1 = GOLDIE, Bit 2 = NINJA |
| `selected` | uint8 | 0 | 0 … 2, muss freigeschaltet sein |
| `sound_on` | bool | true | – |

## 9.2 Anforderungen

| ID | Anforderung |
|----|-------------|
| FR-PS-01 | Daten werden im nichtflüchtigen Speicher des ESP32 (z. B. NVS/Preferences) in einem eigenen Namespace (`schreihuhn`) gespeichert. |
| FR-PS-02 | Laden beim Start (UC-01). Fehlt ein Feld oder ist ein Wert außerhalb des Bereichs, wird **dieses Feld** auf den Default gesetzt. Bit 0 von `unlocked` wird immer erzwungen; ist `selected` nicht freigeschaltet, wird `selected = 0`. |
| FR-PS-03 | Weicht `schema_version` ab (zukünftige Versionen), werden bekannte Felder übernommen, unbekannte ignoriert; kein Absturz. |
| FR-PS-04 | **Schreibzeitpunkte** (nur diese – Flash schonen): Run-Ende (UC-09a), Kauf (UC-04), Run-Start mit geändertem Huhn (UC-05), Ton umschalten (UC-11), Erstinitialisierung. Niemals periodisch während des Spiels. |
| FR-PS-05 | Ein Kauf MUSS Guthaben-Abzug und Freischaltung so speichern, dass nach einem Stromausfall nie "Coins weg, Huhn nicht frei" entstehen kann (z. B. beide Werte in einem Commit; oder erst Freischaltung, dann Abzug). |
| FR-PS-06 | Coins und Highscore werden bei Überschreitung auf das Maximum begrenzt (kein Überlauf). |

## 9.3 Shop-Regeln

| ID | Regel |
|----|-------|
| FR-SH-01 | Preise: HENNI 0 (immer frei), GOLDIE **50**, NINJA **150** Coins (Tuning-Konstanten `K_PRICE_GOLDIE`, `K_PRICE_NINJA`). |
| FR-SH-02 | Kauf nur möglich, wenn `coins ≥ Preis` und Huhn noch gesperrt. |
| FR-SH-03 | Bei Kauf: `coins -= Preis`, Bit setzen, `selected = gekauftes Huhn`, speichern (FR-PS-05). |
| FR-SH-04 | Gekaufte Hühner bleiben dauerhaft frei. Es gibt keinen Verkauf / keine Rückerstattung. |
| FR-SH-05 | Hühner sind **rein kosmetisch** – identische Hitbox, Physik und Werte. |
| FR-SH-06 | Ein "Werkseinstellungs-Reset" ist im MVP nicht über das UI erreichbar; Entwickler können ihn per Build-Flag `RESET_SAVE` auslösen (löscht beim Start den Speicherstand). |
