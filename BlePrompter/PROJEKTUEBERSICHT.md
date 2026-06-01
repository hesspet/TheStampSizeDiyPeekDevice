---
Datum: 01.06.2026
Version: 15
Autor: Peter Heß, Germany (+Codex)
---

# BlePrompter

`BlePrompter` ist eine ESP32-C3-Test-Firmware für das 0,42-Zoll-OLED-Entwicklungsboard aus `../docs/ESP32-C3 -OLED-Entwicklungsboard.description.md`.

## Zweck

Die Firmware zeigt per BLE empfangene Befehle auf dem OLED an. Ziel ist eine einfache externe Steuerung ohne eigene native Smartphone-App. Geeignete Werkzeuge sind Android-Apps wie nRF Connect, Serial Bluetooth Terminal, MacroDroid mit BLE-Plugin und andere BLE-UART-fähige Werkzeuge.

## Hardwarebelegung

| Funktion | GPIO | Hinweis |
| --- | --- | --- |
| OLED SDA | GPIO5 | I2C-Datenleitung |
| OLED SCL | GPIO6 | I2C-Taktleitung |
| Button | GPIO9 | `INPUT_PULLUP`, gedrückt ist `LOW`; langer Druck aktiviert Tiefschlaf |

## Start und Schlafzyklus

Nach Bestromung oder Reset startet die Firmware den zyklischen Tiefschlafmodus mit einem ersten Wachfenster von `10 s`. Dadurch ist BLE direkt nach einem Reset kurz online, kann für einen Zwangs-Reconnect genutzt werden und erleichtert das Flashen. Danach läuft der Zyklus mit `30 s` Schlafdauer und `10 s` Wachfenster weiter.

Bei Timer-Aufwachen aus dem zyklischen Tiefschlaf startet BLE für das Wachfenster. Nach jedem fünften Zyklus wird das Wachfenster auf `60 s` verlängert. Eine BLE-Verbindung pausiert den Zyklus für die Bedienung; nach der Trennung startet die Firmware den zyklischen Tiefschlaf wieder automatisch.

Bei regulären Starts außerhalb des automatischen Power-On-Zyklus zeigt das OLED kurz:

- `BlePrompter`
- die Programmversion
- `Build:`
- das Builddatum im Format `DD.MM.YYYY`

Danach zeigt das OLED den BLE-Bereitschaftsstatus.

## BLE

- Bluetooth-Name: `BlePrompter-xxxx`, wobei `xxxx` aus der ESP32-Chip-ID abgeleitet wird
- Protokoll: BLE-UART kompatibel zum Nordic UART Service
- Service-UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX-Characteristic zum Schreiben: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX-Characteristic für Antworten: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
- Pairing: einfaches BLE-Bonding ohne Passkey.

## Befehle

Die Befehlsdokumentation liegt in `./BEFEHLE.md`.

Unterstützt werden:

- Textanzeige mit `TEXT`
- große Symbolanzeige mit `SYMBOL`
- ESP-Symbole mit `ESP` und Kurzbefehlen `EC`, `EG`, `EW`, `EQ`, `ES`
- Pfeile mit `ARROW`
- Spielkarten mit `CARD`
- Würfelsymbole mit `CUBE` und `CUBES`
- invertierte Darstellung mit `INVERT`
- 180-Grad-Darstellung für kopfüber montierte Displays mit `U1` und `U0`
- Sleep-Modi mit `SLEEP DISPLAY`, `SLEEP DEEP <Sekunden>`, `SLEEP CYCLE [Schlafsekunden Listensekunden]`, `SLEEP RESET` und `WAKE`
- Sleep-Button auf GPIO9: nach 5 Sekunden Halten zählt das OLED von `5` bis `0` und aktiviert zyklischen Tiefschlaf mit `30 s` Schlafdauer und `10 s` Wachfenster
- Löschen der Anzeige mit `CLEAR`; der Clear-Zustand zeigt 2x2-Pixel-Markierungen in allen vier Ecken, damit aktives Display und Sleep unterscheidbar bleiben
- Clients sollen nach `CLEAR`, `CLS` oder `CL` kein `SLEEP DISPLAY` senden, wenn die Eckenmarkierungen sichtbar bleiben sollen
- kurze Hilfe mit `HELP`
- kurze Makro-Aliasse wie `SA`, `SOK`, `EC`, `EW`, `AN`, `ASW`, `CHX`, `CJ1`, `I1`, `I0`, `U1`, `U0`, `CL` und `H`
- Kartenbefehle nutzen englische Pokerbezeichnungen: `Heart`, `Diamond`, `Clubs`, `Spade`, `Ace`, `Jack`, `Queen`, `King` und `X` für 10.

## Sleep-Modi

`SLEEP DISPLAY` schaltet das OLED und den I2C-Bus ab, lässt BLE aber aktiv. Der nächste BLE-Befehl weckt das Display ohne Reset und wird direkt ausgeführt.

`SLEEP DISPLAY` entfernt auch die sichtbaren Clear-Markierungen. Ein Client, der den Clear-Zustand als Verbindungsindikator nutzen möchte, muss nach `CLEAR`, `CLS` oder `CL` verbunden bleiben und darf nicht direkt in Display-Sleep wechseln.

`SLEEP DEEP <Sekunden>` schaltet das OLED ab und wechselt dann in den ESP32-Tiefschlaf mit Timer-Aufwachen. BLE trennt dabei, und die Firmware startet nach Ablauf der Zeit neu.

`SLEEP CYCLE` schaltet das OLED ab und wechselt in einen zyklischen Tiefschlaf. Standard sind `30 s` Schlafdauer und `10 s` Wachfenster. Nach jedem fünften Zyklus wird das Wachfenster auf `60 s` verlängert. Während des Wachfensters zeigt das OLED `BlePrompter`, `Wachfenster`, die Restzeit und die Gerätekennung, zum Beispiel `BP-3F8A`.

`WAKE` beendet den laufenden zyklischen Schlafmodus während einer aktiven Verbindung. Nach einer späteren BLE-Trennung startet die Firmware den zyklischen Tiefschlaf wieder automatisch, damit das Gerät nicht dauerhaft wach bleibt.

`SLEEP RESET` schaltet das OLED ab und wechselt dann in den ESP32-Tiefschlaf ohne Timer. Das Gerät wacht erst durch Reset oder EN wieder auf.

GPIO9 dient zusätzlich als Sleep-Button. Wird der Button länger als 5 Sekunden gehalten, zeigt das OLED `Zykl. Schlaf` und einen Countdown von `5` bis `0`. Wird der Button vorher losgelassen, wird der vorherige OLED-Zustand wiederhergestellt. Nach Ablauf des Countdowns wird zyklischer Tiefschlaf aktiviert; das OLED wird direkt vor dem Tiefschlaf deaktiviert. `SLEEP RESET` bleibt als expliziter Befehl für Reset/EN-Aufwachen erhalten.

## Display-Bibliothek

`BlePrompter` nutzt die gemeinsame Projektbibliothek `../lib/StampDisplay` für:

- Pfeile
- Symbole
- ESP-Symbole
- Würfelsymbole
- Spielkarten

## Debug und Konfiguration

Die Konfiguration liegt in `include/config.h`.

Der Debuglevel ist über `configuredDebugLevel` schaltbar:

- `DebugLevel::none`
- `DebugLevel::info`
- `DebugLevel::debug`
- `DebugLevel::trace`

Auch bei `DebugLevel::none` wird beim Start ein kurzer Programmheader ausgegeben. Debugausgaben gehen über USB-Serial.

## PlatformIO

Build:

```powershell
cd BlePrompter
pio run
```

Upload:

```powershell
cd BlePrompter
pio run --target upload
```

Monitor:

```powershell
cd BlePrompter
pio device monitor --port COM6 --baud 115200
```

Für das aktuell angeschlossene Board ist `COM6` konfiguriert. `ARDUINO_USB_MODE=1` mit `ARDUINO_USB_CDC_ON_BOOT=1` nutzt die ESP32-C3-Hardware-USB-CDC/JTAG-Schnittstelle.