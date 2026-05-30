---
Datum: 26.05.2026
Version: 9
Autor: Peter Heß, Germany (+Codex)
---

# BlePrompter Commands

`BlePrompter` nimmt Befehle über BLE-UART entgegen. Das Profil ist kompatibel zum Nordic UART Service.

## BLE-Verbindung

- Bluetooth-Name: `BlePrompter-xxxx`, wobei `xxxx` aus der ESP32-Chip-ID abgeleitet wird
- Service-UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX-Characteristic zum Schreiben: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX-Characteristic für Antworten: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
- Pairing: einfaches BLE-Bonding ohne Passkey. Apps können sich verbinden und bei Bedarf koppeln.

In Android-Apps wie nRF Connect, Serial Bluetooth Terminal oder MacroDroid-BLE-Plugins werden die Befehle als Text an die RX-Characteristic geschrieben. Ein Zeilenumbruch ist erlaubt, aber nicht erforderlich.

## Antworten

Nach der Verarbeitung eines Befehls sendet die Firmware auf der TX-Characteristic den außen getrimmten Originalbefehl mit Statussuffix zurück.

Format:

```text
<Befehl>:OK
<Befehl>:ERROR
```

Beispiele:

```text
SOK:OK
TEXT Hallo:OK
CARD Foo:ERROR
INVERT MAYBE:ERROR
```

`HELP`, `H` und `?` bleiben Sonderfälle und senden nur die kurze Befehlsübersicht.

## Kurzbefehle für Makros

Alle Befehle sind unabhängig von Groß- und Kleinschreibung. `chx`, `ChX` und `CHX` sind also gleichwertig.

Die Kurzsyntax nutzt eine einheitliche englische Sprachbasis:

| Kurzform | Bedeutung |
| --- | --- |
| `H` | Help senden |
| `CL` | Display löschen |
| `I1` | invertierte Darstellung einschalten |
| `I0` | invertierte Darstellung ausschalten |
| `U1` | 180-Grad-Darstellung einschalten |
| `U0` | Normaldarstellung einschalten |
| `SA` | Symbol `A` anzeigen |
| `SOK` | Symbolpaar `OK` anzeigen |
| `EC` | ESP-Kreis anzeigen |
| `EG` | ESP-Kreuz anzeigen |
| `EW` | ESP-Wellen anzeigen |
| `EQ` | ESP-Quadrat anzeigen |
| `ES` | ESP-Stern anzeigen |
| `AN` | Arrow North anzeigen |
| `ASW` | Arrow Southwest anzeigen |
| `CHX` | Card Heart 10 anzeigen |
| `CJ1` | Joker 1 anzeigen |

## Clear

```text
CLEAR
```

Aliasse:

```text
CLS
CL
```

Löscht die OLED-Anzeige.

## Text

```text
TEXT Hello
```

Zeilen können mit `|` getrennt werden:

```text
TEXT Line 1|Line 2|Line 3|Line 4
```

Das Display ist sehr klein. Gut lesbar sind ungefähr 12 Zeichen pro Zeile und maximal 4 Zeilen.

Alias:

```text
TXT Hello
```

## Symbol

```text
SYMBOL A
SYMBOL OK
```

Aliasse:

```text
SYM OK
SA
SOK
```

Es werden ein oder zwei druckbare ASCII-Zeichen groß und zentriert angezeigt. Symbol-Ausgaben werden immer in Großbuchstaben angezeigt.

## ESP-Symbole

```text
ESP Circle
ESP Cross
ESP Waves
ESP Square
ESP Star
```

Kurzformen:

```text
EC
EG
EW
EQ
ES
```

Die ESP-Symbole entsprechen den klassischen Zener-Kartenformen: Kreis, Kreuz, Wellen, Quadrat und Stern. Sie werden auf dem OLED mit U8g2-Primitiven gezeichnet und unterstützen normale sowie invertierte Darstellung.

## Arrow

```text
ARROW N
ARROW NE
ARROW E
ARROW SE
ARROW S
ARROW SW
ARROW W
ARROW NW
```

Kurzformen:

```text
AN
ANE
AE
ASE
AS
ASW
AW
ANW
```

## Card

Karten können über ihren Index `0` bis `53` angezeigt werden:

```text
CARD 0
CARD 52
```

Alternativ kann Farbe und Wert angegeben werden:

```text
CARD Heart A
CARD Heart X
CARD Diamond 7
CARD Clubs Jack
CARD Spade King
CARD J1
CARD J2
```

Kurzformen beginnen immer mit `C`:

```text
CHX
CD7
CCJ
CCQ
CSK
CJ1
CJ2
```

Aufbau:

- erstes Zeichen `C`: Card
- zweites Zeichen: Suit
- drittes Zeichen: Rank

Suits in Kurzform:

- `H`: Heart
- `D`: Diamond
- `C`: Clubs
- `S`: Spade

Ranks in Kurzform:

- `1` oder `A`: Ace
- `2` bis `9`
- `X`: 10
- `J`: Jack
- `Q`: Queen
- `K`: King
- `J1`: Joker 1 mit komplettem Befehl `CJ1`
- `J2`: Joker 2 mit komplettem Befehl `CJ2`

Suits:

- `Heart`
- `Diamond`
- `Clubs`
- `Spade`

Ranks:

- `1`, `A` oder `Ace`
- `2` bis `9`
- `10` oder `X`
- `J` oder `Jack`
- `Q` oder `Queen`
- `K` oder `King`
- `J1` oder `Joker 1`
- `J2` oder `Joker 2`

## Invert

```text
INVERT ON
INVERT OFF
```

Aliasse:

```text
INV ON
INV OFF
I1
I0
```

Die Einstellung gilt für die folgenden Anzeige-Befehle.

## Upside-down

```text
U1
U0
```

`U1` dreht die OLED-Ausgabe für die folgenden Anzeige-Befehle um 180 Grad. `U0` stellt die Normaldarstellung wieder her.

Die Einstellung ist für Displays gedacht, die kopfüber montiert sind. `U1` und `U0` zeichnen den aktuellen Bildinhalt nicht neu; die geänderte Ausrichtung wird erst beim nächsten Anzeige-Befehl sichtbar.

## Sleep

```text
SLEEP DISPLAY
SLEEP DEEP 60
SLEEP CYCLE
SLEEP CYCLE 30 10
SLEEP RESET
WAKE
```

`SLEEP DISPLAY` schaltet das OLED und den I2C-Bus ab. BLE bleibt aktiv. Der nächste empfangene BLE-Befehl aktiviert das Display sofort wieder und wird direkt ausgeführt.

`SLEEP DEEP <Sekunden>` versetzt das Gerät in Tiefschlaf und aktiviert einen Timer als Aufwachquelle. Danach trennt BLE. Nach Ablauf der Sekunden startet die Firmware regulär neu.

`SLEEP CYCLE` versetzt das Gerät in zyklischen Tiefschlaf. Standard sind `30 s` Schlafdauer und `10 s` Wachfenster. Nach jedem fünften Zyklus bleibt das Gerät `60 s` erreichbar, damit Smartphone-Scans das Gerät sicherer finden.

`SLEEP CYCLE <Schlafsekunden> <Listensekunden>` nutzt eigene Werte. Erlaubt sind `5` bis `60` Sekunden Schlafdauer und `10` bis `120` Sekunden Listenzeit. Während eines Wachfensters zeigt das OLED `BlePrompter`, `Wachfenster`, die Restzeit und die Gerätekennung.

`WAKE` beendet den zyklischen Schlafmodus nach einer Verbindung und hält das Gerät aktiv.

`SLEEP RESET` versetzt das Gerät in Tiefschlaf ohne Timer. Danach trennt BLE. Das Gerät wacht erst durch Reset oder EN wieder auf.

## Help

```text
HELP
```

Aliasse:

```text
?
H
```

Sendet eine kurze Befehlsübersicht über die TX-Characteristic zurück.

## Beispiele für MacroDroid

Für MacroDroid kann ein BLE-Plugin genutzt werden, das den Nordic UART Service beschreiben kann. Jede Aktion sendet dann genau einen Textbefehl, zum Beispiel:

```text
TEXT Door|open
ARROW NE
SYMBOL OK
ESP Circle
EW
SLEEP DISPLAY
CLEAR
ANE
SOK
CHX
I1
I0
CL
```

Für Makro-Buttons in Serial Bluetooth Terminal eignen sich dieselben Befehle.
