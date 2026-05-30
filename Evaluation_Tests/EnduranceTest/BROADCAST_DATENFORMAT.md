---
Datum: 23.05.2026
Version: 1
Autor: Peter Heß, Germany (+Codex)
---
# Broadcast-Datenformat

Diese Datei beschreibt das UDP-Datenformat von `EnduranceTest`. Sie dient als Anleitung für einen späteren Task, der einen Python-Empfänger schreiben soll.

## Transport

- Protokoll: UDP-Broadcast.
- Zieladresse: `255.255.255.255`.
- Zielport: `4210`.
- Zeichensatz: UTF-8.
- Nachrichtenformat: JSON Lines.
- Eine UDP-Nachricht enthält genau ein JSON-Objekt ohne zusätzliche Binärdaten.

Der Empfänger soll auf `0.0.0.0:4210` lauschen, jedes empfangene UDP-Paket als UTF-8 dekodieren, als JSON parsen und die Objekte zeilenweise in eine Datei schreiben.

## Pakettypen

### `cycle_start`

Wird vor den eigentlichen Messdatensätzen gesendet.

Beispiel:

```json
{"type":"cycle_start","program":"EnduranceTest","version":"1.0.0","cycle":7,"uptime_ms":431000,"record_count":35}
```

Felder:

| Feld | Bedeutung |
| --- | --- |
| `type` | Immer `cycle_start`. |
| `program` | Programmname der Firmware. |
| `version` | Firmware-Version. |
| `cycle` | Laufende Zyklusnummer seit Boot. |
| `uptime_ms` | Zeitstempel in Millisekunden seit Boot. |
| `record_count` | Anzahl folgender `log`-Pakete. |

### `log`

Enthält einen einzelnen Protokolleintrag aus dem RAM-Puffer.

Beispiel:

```json
{"type":"log","program":"EnduranceTest","version":"1.0.0","cycle":7,"uptime_ms":392500,"cycle_ms":22500,"action":"wifi_scan","result":"ok","detail":"Netze: 4, stärkstes: Agathas-Netz-16, RSSI: -61","value":4}
```

Felder:

| Feld | Bedeutung |
| --- | --- |
| `type` | Immer `log`. |
| `program` | Programmname der Firmware. |
| `version` | Firmware-Version. |
| `cycle` | Zyklusnummer seit Boot. |
| `uptime_ms` | Zeitstempel in Millisekunden seit Boot. |
| `cycle_ms` | Millisekunden seit Beginn des aktuellen Testzyklus. |
| `action` | Aktion, zum Beispiel `ble_scan`, `display`, `wifi_scan`, `usage`, `progress`, `wifi_connect` oder `cycle`. |
| `result` | Ergebnis, zum Beispiel `ok`, `start`, `done` oder `error`. |
| `detail` | Kurzer menschenlesbarer Detailtext. |
| `value` | Numerischer Mess- oder Statuswert. Bedeutung hängt von `action` ab. |

### `cycle_end`

Wird nach den Log-Paketen gesendet.

Beispiel:

```json
{"type":"cycle_end","program":"EnduranceTest","version":"1.0.0","cycle":7,"uptime_ms":432200}
```

Felder:

| Feld | Bedeutung |
| --- | --- |
| `type` | Immer `cycle_end`. |
| `program` | Programmname der Firmware. |
| `version` | Firmware-Version. |
| `cycle` | Zyklusnummer seit Boot. |
| `uptime_ms` | Zeitstempel in Millisekunden seit Boot. |

## Zeitstempel

Die Firmware hat ohne externe Zeitsynchronisation keine echte Uhrzeit. Deshalb werden alle Zeitstempel als Millisekunden seit Boot gesendet:

- `uptime_ms`: globale Laufzeit seit Start der Firmware.
- `cycle_ms`: Laufzeit innerhalb des aktuellen Testzyklus.

Ein Python-Empfänger kann beim Empfang zusätzlich die lokale Rechnerzeit ergänzen, zum Beispiel als ISO-8601-Zeitstempel.

## Ablageempfehlung für den Python-Empfänger

Der Empfänger sollte je empfangenem Paket eine Zeile in eine `.jsonl`-Datei schreiben. Das ist robust, einfach appendbar und später gut mit Python, `jq` oder Tabellenwerkzeugen auswertbar.

Empfohlene Ergänzungen durch den Empfänger:

- `received_at`: lokale Empfangszeit des Rechners.
- `sender_ip`: IP-Adresse des ESP32.
- `sender_port`: UDP-Absenderport.
