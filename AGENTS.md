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
- Ausnahme: U8g2 wird bewusst per PlatformIO `lib_deps` genutzt, weil die Bibliothek groß ist.
- Ausnahme: `BlePrompter` nutzt `h2zero/NimBLE-Arduino@^1.4.3` per PlatformIO `lib_deps`, weil die BLE-Stack-Abhängigkeit projektspezifisch und reproduzierbar versioniert sein soll.
- Ein Updateverfahren für Bibliotheken muss in `./PROJEKTUEBERSICHT.md` beschrieben sein.

## Aktueller Stand: BlePrompter

Unterprojekt: `./BlePrompter`

Zweck:

- BLE-UART-Firmware für das ESP32-C3-OLED-Board.
- Externe Steuerung ohne native Smartphone-App.
- Geeignet für Android-Apps, MacroDroid mit BLE-Plugin und BLE-Terminal-Apps.
- OLED-Anzeige für Text, Symbole, Pfeile und Spielkarten.
- Kein automatischer Testmodus.
- Startbild mit `BlePrompter`, Versionsnummer und Builddatum.

Wichtige Dateien:

- `BlePrompter/platformio.ini`
- `BlePrompter/include/config.h`
- `BlePrompter/src/main.cpp`
- `BlePrompter/PROJEKTUEBERSICHT.md`
- `BlePrompter/BEFEHLE.md`

BLE-Konfiguration:

- Bluetooth-Name: `BlePrompter`
- Protokoll: BLE-UART kompatibel zum Nordic UART Service
- Service-UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX-Characteristic zum Schreiben: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX-Characteristic für Antworten: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
- Pairing: einfaches BLE-Bonding ohne Passkey.

Befehlssyntax:

- Die Syntax ist bewusst englisch.
- Langbefehle: `TEXT`, `SYMBOL`, `ARROW`, `CARD`, `INVERT`, `CLEAR`, `HELP`.
- Kurzbefehle: `SA`, `SOK`, `AN`, `ANE`, `ASW`, `CHX`, `CD7`, `CCJ`, `CSK`, `CJ1`, `I1`, `I0`, `CL`, `H`.
- `C` steht in Kurzbefehlen für `CARD`.
- `A` steht in Kurzbefehlen für `ARROW`.
- `S` steht in Kurzbefehlen für `SYMBOL`.
- `H` steht in Kurzbefehlen für `HELP`.
- Bei Karten bleibt `X` die sichtbare 10.

## Aktueller Stand: BoardTest

Unterprojekt: `./BoardTest`

Zweck:

- Test-Firmware für das ESP32-C3-OLED-Board.
- OLED-Test mit Startanzeige, Builddatum und Programmversion.
- GPIO9 startet und stoppt einen automatischen Testmodus.
- Selbst gezeichnete Pfeile für acht Kompassrichtungen.
- Anzeige von ein oder zwei ASCII-Zeichen.
- Anzeige eines vollständigen Pokerdecks inklusive Joker.
- Normale und invertierte Anzeige mit hellem Hintergrund.

Wichtige Dateien:

- `BoardTest/platformio.ini`
- `BoardTest/include/config.h`
- `BoardTest/src/main.cpp`
- `BoardTest/PROJEKTUEBERSICHT.md`
- `lib/StampDisplay/include/StampDisplay/ArrowDisplay.h`
- `lib/StampDisplay/include/StampDisplay/AsciiCharacterDisplay.h`
- `lib/StampDisplay/include/StampDisplay/PlayingCardDisplay.h`
- `lib/StampDisplay/src/ArrowDisplay.cpp`
- `lib/StampDisplay/src/AsciiCharacterDisplay.cpp`
- `lib/StampDisplay/src/PlayingCardDisplay.cpp`

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

- Header: `lib/StampDisplay/include/StampDisplay/PlayingCardDisplay.h`
- Implementierung: `lib/StampDisplay/src/PlayingCardDisplay.cpp`

API:

- `PlayingCardDisplay::cardCount` ist `54`.
- `drawCard(display, cardIndex, inverted)` zeichnet eine Karte normal oder invertiert.
- `getCardDescription(cardIndex, buffer, bufferSize)` liefert eine englische Pokerbeschreibung für Debugausgaben und BLE-Kontext.

Deck:

- `0` bis `51`: normales Pokerdeck.
- `52`: Joker 1, Anzeige `J1`.
- `53`: Joker 2, Anzeige `J2`.

Reihenfolge:

- Erst Heart, dann Diamond, Clubs, Spade.
- Pro Farbe: `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`, `X`, `J`, `Q`, `K`.
- `1` steht für Ace.
- `X` steht für 10.
- `J` steht für Jack.
- `Q` steht für Queen.
- `K` steht für King.

Darstellung:

- Reihenfolge auf dem Display ist Suit zuerst, dann Wert.
- Beispiel: Heart 10 wird als großes Heart-Symbol links und `X` rechts angezeigt.
- Die Suit-Symbole werden nicht mehr aus Fonts gezeichnet, sondern mit U8g2-Primitiven selbst gerendert, weil die Font-Symbole auf dem kleinen Display zu schlecht lesbar waren.
- Clubs und Spade haben einen deutlich sichtbaren Stengel, damit sie auf dem monochromen Display besser von Heart unterscheidbar sind.

## Pfeil- und ASCII-Anzeige

- `ArrowDisplay` zeichnet Pfeile für `N`, `NO`, `O`, `SO`, `S`, `SW`, `W` und `NW` selbst mit U8g2-Primitiven.
- `AsciiCharacterDisplay` zeichnet ein oder zwei druckbare ASCII-Zeichen groß und zentriert.
- Beide Klassen unterstützen normale und invertierte Darstellung.

## Bedienlogik

- Nach dem Start zeigt das OLED kurz `BoardTest`, `V 1.1.0`, `Build:` und das Builddatum.
- Danach zeigt das OLED `Test bereit` und `IO9 Start`.
- Ein Druck auf GPIO9 startet den automatischen Testmodus.
- Während der Testmodus läuft, stoppt ein weiterer Druck auf GPIO9 die Sequenz.
- Jede Darstellung bleibt `1000 ms` sichtbar.
- Normale Sequenz:
  - alle Pfeile in den acht Kompassrichtungen
  - 16 zufällige Spielkarten inklusive möglicher Joker
  - Zähler `1` bis `12`
  - 10 zufällige ASCII-Einzelzeichen
  - 10 zufällige ASCII-Zeichenpaare
- Danach folgt dieselbe Sequenz invertiert mit hellem Hintergrund.
- Nach der invertierten Sequenz beginnt der Ablauf wieder von vorne.

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
