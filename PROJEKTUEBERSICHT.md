---
Datum: 26.05.2026
Version: 9
Autor: Peter Heß, Germany (+Codex)
---
# Projektübersicht

Projektname: The Stamp Size Diy Peek Device

Dieses Repository sammelt mehrere PlatformIO-Unterprojekte für das kleine ESP32-C3-OLED-Entwicklungsboard. Die generische Hardwarebeschreibung liegt unter [docs/ESP32-C3 -OLED-Entwicklungsboard.description.md](docs/ESP32-C3%20-OLED-Entwicklungsboard.description.md).

## Entwicklungsumgebung

- PlatformIO
- ESP32 Arduino Framework
- Boardprofil: `esp32-c3-devkitm-1`
- OLED: SSD1306, 72 x 40 Pixel, I2C über GPIO5 und GPIO6
- Board-Button: GPIO9 mit `INPUT_PULLUP`, gedrückt ist `LOW`
- Serial Monitor: 115200 Baud über USB-CDC

## Unterprojekte

| Unterprojekt | Zweck |
| --- | --- |
| [BoardTest](BoardTest/PROJEKTUEBERSICHT.md) | Test-Firmware `1.2.0` für OLED, Buildanzeige, GPIO9-gesteuerten automatischen Testmodus, projektweite Display-Library, selbst gezeichnete Pfeile, ASCII-Zeichenanzeige, Spielkarten und invertierte Anzeige des ESP32-C3-OLED-Boards. |
| [BlePrompter](BlePrompter/PROJEKTUEBERSICHT.md) | BLE-UART-Firmware `1.4.0` mit Nordic UART Service für Android-Apps, MacroDroid/BLE-Plugins und Terminal-Makros. Zeigt Text, Pfeile, Symbole und Spielkarten auf dem OLED an und unterstützt Display-Schlaf sowie Tiefschlaf-Kommandos. |
| [BlePrompterJsClient](BlePrompterJsClient/PROJEKTUEBERSICHT.md) | Statische responsive Web-Bluetooth-Anwendung für GitHub Pages, lokale Demo-Server-Nutzung und BLE-UART-Steuerung von `BlePrompter` mit Pfeilen, Karten und Symbolen. |
| [EnduranceTest](EnduranceTest/PROJEKTUEBERSICHT.md) | Dauertest-Firmware `1.0.0` für BLE-Scan, wechselnde OLED-Anzeigen, WLAN-Scan, simulierte Nutzung und UDP-Broadcast der RAM-Protokolldaten nach jedem Testzyklus. |
| [EnduranceTestClient](EnduranceTestClient/PROJEKTUEBERSICHT.md) | Python-Client zum dauerhaften Empfang der `EnduranceTest`-UDP-Broadcasts und JSONL-Protokollierung im Temp-Verzeichnis. |

## BlePrompter als BLE-Device

`BlePrompter` ist das Unterprojekt für externe Steuerung per BLE. Es eignet sich als Zielgerät für:

- Android-Apps wie nRF Connect oder Serial Bluetooth Terminal.
- MacroDroid über ein BLE-Plugin.
- Eine spätere JavaScript/Web-Bluetooth-Seite im Browser.

BLE-Kenndaten:

- Bluetooth-Name: `BlePrompter`
- Nordic-UART-Service: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX-Characteristic zum Schreiben: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX-Characteristic für Antworten: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`

Die Befehlssyntax ist englisch und makrotauglich. Beispiele: `TEXT Door|open`, `SYMBOL OK`, `ARROW NE`, `CHX`, `I1`, `I0`, `SLEEP DISPLAY`, `SLEEP DEEP 60`, `SLEEP RESET` und `CL`. Für Web-Bluetooth-Implementierungen liegt ein eigener Einstieg unter [BlePrompter/WEB_BLUETOOTH.md](BlePrompter/WEB_BLUETOOTH.md).

## BlePrompterJsClient

`BlePrompterJsClient` ist die statische Browser-Oberfläche für `BlePrompter`. Sie nutzt Web Bluetooth und sendet die bestehenden BLE-UART-Textbefehle an die Firmware.

Wichtige Eigenschaften:

- Direkte Nutzung ohne Build-Schritt.
- Geeignet für GitHub Pages.
- Responsives Design für Desktop, Smartphone und Android-Layouttests.
- Lokaler Demo-Server per `BlePrompterJsClient\start-demo-server.bat`.
- Drei Betriebsarten: Pfeile, Karten und Symbole.
- Web Bluetooth funktioniert für echte BLE-Verbindungen nur in sicheren Kontexten, also über HTTPS oder `localhost`.

## Projektweite Libraries

| Library | Zweck |
| --- | --- |
| [StampDisplay](lib/StampDisplay/PROJEKTUEBERSICHT.md) | Lokale PlatformIO-Library für wiederverwendbare Display-Darstellungsklassen: Pfeile, Symbole und Spielkarten. |

## Projektregeln

- Jedes Unterprojekt enthält eine eigene `PROJEKTUEBERSICHT.md`.
- Jedes ESP32-Programm enthält eine `config.h`.
- Debugausgaben sind in `config.h` über `none`, `info`, `debug` und `trace` schaltbar.
- Auch bei Debuglevel `none` wird beim Start ein kurzer Programmheader mit Builddatum und Konfiguration ausgegeben.
- User-facing Strings und Dokumentation sind deutsch.
- Variablen- und Methodennamen sind verständlich und ausgeschrieben.
- Datumsangaben werden im Format `DD.MM.YYYY` geschrieben.
- OTA wird nicht vorbereitet, solange es nicht explizit gefordert ist.

## Bibliotheken und Updates

Bibliotheken sollen nach Möglichkeit projektlokal versioniert werden. Projektweit wiederverwendbare eigene Klassen liegen unter `lib/` als lokale PlatformIO-Libraries. Die Library `StampDisplay` enthält die Display-Darstellungsklassen, die bisher direkt im Unterprojekt `BoardTest` lagen.

Für U8g2 wurde am 19.05.2026 bewusst entschieden, die Bibliothek wegen ihrer Größe nicht in `lib/` zu übernehmen. Die Unterprojekte verwenden deshalb die in der Boardbeschreibung empfohlene PlatformIO-Abhängigkeit `olikraus/U8g2@^2.36.12`.

Für `BlePrompter` wird `h2zero/NimBLE-Arduino@^1.4.3` per PlatformIO `lib_deps` genutzt. Grund ist, dass die BLE-Stack-Abhängigkeit projektspezifisch ist und PlatformIO sie reproduzierbar versioniert herunterladen kann.

Updateverfahren:

1. Gewünschte Bibliotheksversion in der jeweiligen `platformio.ini` anpassen.
2. `pio run` im Unterprojekt ausführen.
3. Firmware auf dem Board prüfen.
4. Versionsänderung und relevante Hinweise in der Unterprojekt-Übersicht dokumentieren.
