# AGENTS.md

## Lokalisierung

- Alle user-facing Strings müssen lokalisiert werden.
- Deutsche Texte sollen deutsche Umlaute verwenden.
- Datumsformate müssen der EU-Norm entsprechen: `DD.MM.YYYY`.
- Dokumentation und Codedokumentation werden auf Deutsch geschrieben.
- Methoden, Variablen und Klassennamen sollen verständliche englische Namen ohne unnötige Abkürzungen bekommen.

## Projektkontext

- Projektname: `The Stamp Size Diy Peek Device`
- Entwicklungsumgebung: PlatformIO für ESP32 mit Arduino Framework.
- Das Projektverzeichnis kann mehrere PlatformIO-Unterprojekte enthalten.
- Generischer Projektkontext liegt in `./PROJEKTUEBERSICHT.md`.
- Jedes Unterprojekt muss eine eigene `PROJEKTUEBERSICHT.md` besitzen.
- Technische Hardwarebeschreibung: `./docs/ESP32-C3 -OLED-Entwicklungsboard.description.md`

Die Hardwarebeschreibung enthält:

- Displayansteuerung.
- Buttonbeschreibung.
- Sample-Code.
- PlatformIO-Setup-Hinweise.
- Board-spezifische Hinweise.

## ESP32-Regeln

- Aktuell wird keine OTA-Technik geplant, außer sie wird explizit gefordert.
- PlatformIO soll so konfiguriert werden, dass möglichst viel Programmspeicher verfügbar bleibt.
- Jedes ESP32-Programm muss eine `config.h` enthalten.
- Debugmechanismus in `config.h` schaltbar über `none`, `info`, `debug`, `trace`.
- Bei `none` soll beim Start trotzdem immer ein kurzer Programmheader mit Compiledatum, Name und ggf. aktueller Konfiguration ausgegeben werden.
- Wenn ein Projekt mit WiFi arbeitet, immer ein Konfigurationssystem mit AP vorsehen. Dafür sollen nach Möglichkeit fertige Frameworks genutzt werden.
- Projektbibliotheken sollen grundsätzlich unter `./lib` heruntergeladen und ins Git übernommen werden.
- Ausnahme: Für `BoardTest` wird U8g2 bewusst per PlatformIO `lib_deps` genutzt, weil die Bibliothek groß ist.
- Ein Updateverfahren für Bibliotheken muss in `./PROJEKTUEBERSICHT.md` beschrieben sein.

## Aktueller Stand: BoardTest

Unterprojekt: `./BoardTest`

Zweck:

- Test-Firmware für das ESP32-C3-OLED-Board.
- OLED-Test mit Buildanzeige.
- GPIO9-Button-Test.
- Anzeige großer Pfeile.
- Anzeige eines vollständigen Pokerdecks inklusive Joker.

Wichtige Dateien:

- `BoardTest/platformio.ini`
- `BoardTest/include/config.h`
- `BoardTest/include/PlayingCardDisplay.h`
- `BoardTest/src/PlayingCardDisplay.cpp`
- `BoardTest/src/main.cpp`
- `BoardTest/PROJEKTUEBERSICHT.md`

Hardwarebelegung:

- OLED SDA: GPIO5
- OLED SCL: GPIO6
- Button: GPIO9, `INPUT_PULLUP`, gedrückt ist `LOW`
- Display: SSD1306, 72 x 40 Pixel

PlatformIO-Konfiguration:

- Boardprofil: `esp32-c3-devkitm-1`
- Framework: Arduino
- Upload-Port: `COM6`
- Monitor-Port: `COM6`
- Monitor-Speed: `115200`
- Build-Flags:
  - `ARDUINO_USB_MODE=1`
  - `ARDUINO_USB_CDC_ON_BOOT=1`
- Library:
  - `olikraus/U8g2@^2.36.12`

USB/COM-Erkenntnis:

- Die schlanke PlatformIO-Konfiguration ist bewusst an `C:\dev\AnwesenheitsAnzeige\firmware\AnwesenheitsAnzeige.Esp32` angeglichen.
- Keine `Serial1.begin(...)`-Initialisierung verwenden.
- Keine aggressiven Monitor-DTR/RTS-Sonderkonfigurationen verwenden.
- Mit der aktuellen Konfiguration bleibt `COM6` nach dem Flashen und Reset wieder sichtbar.
- Falls `COM6` mit `PermissionError(13, Zugriff verweigert)` blockiert ist, läuft sehr wahrscheinlich noch ein Serial-Monitor-Prozess.

## Kartenanzeige

Die Kartenanzeige ist in einer isolierten Klasse umgesetzt:

- Header: `BoardTest/include/PlayingCardDisplay.h`
- Implementierung: `BoardTest/src/PlayingCardDisplay.cpp`

API:

- `PlayingCardDisplay::cardCount` ist `54`.
- `drawCard(display, cardIndex)` zeichnet eine Karte.
- `getCardDescription(cardIndex, buffer, bufferSize)` liefert eine deutsche Beschreibung für Debugausgaben.

Deck:

- `0` bis `51`: normales Pokerdeck.
- `52`: Joker 1, Anzeige `J1`.
- `53`: Joker 2, Anzeige `J2`.

Reihenfolge:

- Erst Herz, dann Karo, Kreuz, Pik.
- Pro Farbe: `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`, `X`, `J`, `Q`, `K`.
- `1` steht für Ass.
- `X` steht für 10.
- `J` steht für Jack/Bube.
- `Q` steht für Queen/Dame.
- `K` steht für King/König.

Darstellung:

- Reihenfolge auf dem Display ist Suit zuerst, dann Wert.
- Beispiel: Herz 10 wird als großes Herz links und `X` rechts angezeigt.
- Die Suit-Symbole werden nicht mehr aus Fonts gezeichnet, sondern mit U8g2-Primitiven selbst gerendert, weil die Font-Symbole auf dem kleinen Display zu schlecht lesbar waren.

## Bedienlogik

- Nach dem Start zeigt das OLED kurz `BoardTest`, `Build:` und das Builddatum.
- Danach laufen die Testbilder über den Button GPIO9.
- Reihenfolge:
  - Pfeil nach oben
  - Pfeil nach unten
  - Pfeil nach links
  - Pfeil nach rechts
  - vollständiges Pokerdeck inklusive `J1` und `J2`
  - danach wieder von vorne

## Nützliche Befehle

Build:

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\BoardTest
& "$env:APPDATA\Python\Python313\Scripts\pio.exe" run
```

Upload:

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\BoardTest
& "$env:APPDATA\Python\Python313\Scripts\pio.exe" run --target upload
```

Ports prüfen:

```powershell
python -m serial.tools.list_ports -v
```

Monitor:

```powershell
cd C:\dev\TheStampSizeDiyPeekDevice\BoardTest
& "$env:APPDATA\Python\Python313\Scripts\pio.exe" device monitor --port COM6 --baud 115200
```
