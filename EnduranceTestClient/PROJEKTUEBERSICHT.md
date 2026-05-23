---
Datum: 23.05.2026
Version: 1
Autor: Peter Heß, Germany (+Codex)
---
# EnduranceTestClient

`EnduranceTestClient` empfängt die UDP-Broadcast-Daten der Firmware `EnduranceTest`.

## Zweck

- Lauscht dauerhaft auf UDP-Port `4210`.
- Empfängt Broadcast-Pakete von `EnduranceTest`.
- Schreibt die empfangenen Daten in eine JSONL-Protokolldatei im Temp-Verzeichnis.
- Öffnet und schließt die Protokolldatei nach jedem Empfangsblock.
- Gibt empfangene Daten, Absender, Zeitstempel und Protokolldateipfad auf der Console aus.
- Läuft endlos bis zum Abbruch mit `Ctrl+C` beziehungsweise `Strg+C`.

## Dateien

- `receive_endurance_broadcast.py`: Python-Empfänger.
- `start-client.bat`: Batchdatei zum Starten des Empfängers.
- `PROJEKTUEBERSICHT.md`: Projektkontext.

## Datenformat

Die Protokolldatei wird im Temp-Verzeichnis angelegt. Der Dateiname hat das Muster:

```text
endurance_test_YYYYMMDD_HHMMSS.jsonl
```

Jede Zeile enthält genau ein JSON-Objekt mit:

- `received_at`: lokaler Empfangszeitstempel mit Zeitzone.
- `sender_ip`: IP-Adresse des Absenders.
- `sender_port`: UDP-Absenderport.
- `raw`: empfangener UTF-8-Text.
- `data`: geparstes JSON-Objekt, falls das Paket gültiges JSON enthält.
- `parse_error`: Fehlertext, falls das Paket nicht als JSON geparst werden kann.

## Start

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\EnduranceTestClient
.\start-client.bat
```

Alternativ direkt:

```powershell
python receive_endurance_broadcast.py
```

Der Client läuft endlos und wird mit `Strg+C` beendet.
