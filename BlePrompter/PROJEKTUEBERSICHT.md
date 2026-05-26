---
Datum: 26.05.2026
Version: 10
Autor: Peter Heß, Germany (+Codex)
---

# BlePrompter

`BlePrompter` ist eine ESP32-C3-Test-Firmware für das 0,42-Zoll-OLED-Entwicklungsboard aus `../docs/ESP32-C3 -OLED-Entwicklungsboard.description.md`.

## Zweck

Die Firmware zeigt per BLE empfangene Befehle auf dem OLED an. Ziel ist eine einfache externe Steuerung ohne eigene native Smartphone-App. Geeignete Werkzeuge sind Android-Apps wie nRF Connect, Serial Bluetooth Terminal, MacroDroid mit BLE-Plugin und eine spätere JavaScript-Seite über Web Bluetooth.

## Hardwarebelegung

| Funktion | GPIO | Hinweis |
| --- | --- | --- |
| OLED SDA | GPIO5 | I2C-Datenleitung |
| OLED SCL | GPIO6 | I2C-Taktleitung |
| Button | GPIO9 | `INPUT_PULLUP`, gedrückt ist `LOW`; langer Druck aktiviert Tiefschlaf |

## Startanzeige

Nach dem Start zeigt das OLED kurz:

- `BlePrompter`
- die Programmversion
- `Build:`
- das Builddatum im Format `DD.MM.YYYY`

Danach zeigt das OLED den BLE-Bereitschaftsstatus.

## BLE

- Bluetooth-Name: `BlePrompter`
- Protokoll: BLE-UART kompatibel zum Nordic UART Service
- Service-UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX-Characteristic zum Schreiben: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX-Characteristic für Antworten: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
- Pairing: einfaches BLE-Bonding ohne Passkey.

## Web-Bluetooth-Hinweise

Eine JavaScript-Seite soll das Gerät über `navigator.bluetooth.requestDevice(...)` auswählen, den Nordic-UART-Service öffnen und Textbefehle per `TextEncoder` in die RX-Characteristic schreiben. Die Web-Bluetooth-Implementierung muss den Service in `optionalServices` angeben, weil der Browser sonst nach der Geräteauswahl keinen Zugriff auf den GATT-Service gewährt.

Wichtige Implementierungsdetails:

- Gerätefilter: `name: "BlePrompter"` oder `namePrefix: "BlePrompter"`.
- Service-UUID: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`.
- Schreib-Characteristic: `6e400002-b5a3-f393-e0a9-e50e24dcca9e`.
- Antwort-Characteristic: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`.
- Befehle werden als UTF-8-Text gesendet, zum Beispiel `CHX`, `ARROW NE` oder `TEXT Door|open`.
- Nach der Verarbeitung sendet die Firmware `<Befehl>:OK` oder `<Befehl>:ERROR` als UTF-8-Text zurück. Der Befehlsteil ist der außen getrimmte Originalbefehl; `HELP`, `H` und `?` senden nur die Befehlsübersicht.
- Für Browser ist HTTPS oder `localhost` relevant, weil Web Bluetooth nur in sicheren Kontexten verfügbar ist.

Der konkrete JavaScript-Einstieg liegt in `./WEB_BLUETOOTH.md`.

## Befehle

Die Befehlsdokumentation liegt in `./BEFEHLE.md`. Web-Bluetooth-spezifische Hinweise liegen in `./WEB_BLUETOOTH.md`.

Unterstützt werden:

- Textanzeige mit `TEXT`
- große Symbolanzeige mit `SYMBOL`
- ESP-Symbole mit `ESP` und Kurzbefehlen `EC`, `EG`, `EW`, `EQ`, `ES`
- Pfeile mit `ARROW`
- Spielkarten mit `CARD`
- invertierte Darstellung mit `INVERT`
- 180-Grad-Darstellung für kopfüber montierte Displays mit `U1` und `U0`
- Sleep-Modi mit `SLEEP DISPLAY`, `SLEEP DEEP <Sekunden>` und `SLEEP RESET`
- Sleep-Button auf GPIO9: nach 5 Sekunden Halten zählt das OLED von `5` bis `0` und aktiviert Tiefschlaf bis Reset/EN
- Löschen der Anzeige mit `CLEAR`
- kurze Hilfe mit `HELP`
- kurze Makro-Aliasse wie `SA`, `SOK`, `EC`, `EW`, `AN`, `ASW`, `CHX`, `CJ1`, `I1`, `I0`, `U1`, `U0`, `CL` und `H`
- Kartenbefehle nutzen englische Pokerbezeichnungen: `Heart`, `Diamond`, `Clubs`, `Spade`, `Ace`, `Jack`, `Queen`, `King` und `X` für 10.

## Sleep-Modi

`SLEEP DISPLAY` schaltet das OLED und den I2C-Bus ab, lässt BLE aber aktiv. Der nächste BLE-Befehl weckt das Display ohne Reset und wird direkt ausgeführt.

`SLEEP DEEP <Sekunden>` schaltet in den ESP32-Tiefschlaf mit Timer-Aufwachen. BLE trennt dabei, und die Firmware startet nach Ablauf der Zeit neu.

`SLEEP RESET` schaltet in den ESP32-Tiefschlaf ohne Timer. Das Gerät wacht erst durch Reset oder EN wieder auf.

GPIO9 dient zusätzlich als reiner Sleep-Button. Wird der Button länger als 5 Sekunden gehalten, zeigt das OLED `Tiefschlaf` und einen Countdown von `5` bis `0`. Wird der Button vorher losgelassen, wird der vorherige OLED-Zustand wiederhergestellt. Nach Ablauf des Countdowns wird derselbe Reset-Tiefschlaf wie bei `SLEEP RESET` aktiviert. GPIO9 wird bewusst nicht als Deep-Sleep-Weckquelle genutzt, weil die ESP32-C3-SDK-Unterstützung für Deep-Sleep-GPIO-Wakeup nur GPIO0 bis GPIO5 zulässt; Aufwachen erfolgt deshalb über Reset oder EN.

## Display-Bibliothek

`BlePrompter` nutzt die gemeinsame Projektbibliothek `../lib/StampDisplay` für:

- Pfeile
- Symbole
- ESP-Symbole
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
