---
Datum: 19.05.2026
Version: 1
Autor: Peter Heß, Germany (+Codex)
---
# ESP32-C3-OLED-Entwicklungsboard

Stand: 19.05.2026

Diese Datei beschreibt die Integration des kleinen ESP32-C3-OLED-Entwicklungsboards für die Wiederverwendung in anderen ESP32-Projekten. Der Fokus liegt auf Board, Display, Button, PlatformIO, USB-Serial und typischen Stolperstellen.

## Boardprofil

- Mikrocontroller: ESP32-C3, in PlatformIO als `esp32-c3-devkitm-1` konfiguriert.
- Framework: Arduino über PlatformIO.
- Anzeige: integriertes 0,42-Zoll-OLED mit SSD1306-Controller, 72 x 40 Pixel.
- Grafikbibliothek: `olikraus/U8g2`.
- Display-Bus: I2C.
- Display-SDA: GPIO5.
- Display-SCL: GPIO6.
- Button: GPIO9, intern per `INPUT_PULLUP` hochgezogen, gedrückt bedeutet `LOW`.
- Optionaler Sensoreingang im Beispielaufbau: GPIO4, `HIGH` bedeutet Person erkannt.
- Optionale Status-LED im Beispielaufbau: GPIO8, aktiv bei `LOW`.
- Serial Monitor: 115200 Baud über USB-CDC.

Das Board ist für kleine Statusanzeigen gut geeignet. Die nutzbare Bildschirmfläche ist sehr klein, deshalb funktionieren kurze Texte, Statussymbole und zweizeilige Anzeigen deutlich besser als längere Meldungen.

## PlatformIO-Grundkonfiguration

Die bewährte `platformio.ini` für dieses Board:

```ini
[env:esp32c3]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
upload_port = COM3
monitor_port = COM3
monitor_speed = 115200
build_flags =
    -D ARDUINO_USB_MODE=1
    -D ARDUINO_USB_CDC_ON_BOOT=1
lib_deps =
    olikraus/U8g2@^2.36.12
```

Wichtige Hinweise:

- `upload_port` und `monitor_port` müssen zum lokalen Windows-COM-Port passen.
- Im getesteten Setup war das Board auf `COM3` erreichbar.
- `monitor_speed = 115200` muss zu `Serial.begin(115200)` passen.
- Die beiden `build_flags` sind für zuverlässige Serial-Ausgaben über USB-C auf dem ESP32-C3 wichtig.
- Wenn `pio` nicht im normalen `PATH` liegt, kann PlatformIO trotzdem installiert sein. Im genutzten Setup lag `pio.exe` unter `%APPDATA%\Python\Python313\Scripts\pio.exe`.

Typische Befehle:

```powershell
cd firmware/ESP32-OledBoard
pio run
pio run --target upload
pio device monitor --port COM3 --baud 115200
```

Wenn der Upload mit `Could not open COM3` oder `Zugriff verweigert` fehlschlägt, ist der Port meist noch durch den Serial Monitor, VS Code, Arduino IDE oder ein anderes Programm belegt.

## OLED- und Grafikintegration

Das Board nutzt ein SSD1306-OLED mit 72 x 40 Pixeln. Im getesteten Beispiel wird U8g2 im Full-Buffer-Modus mit Hardware-I2C verwendet:

```cpp
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

constexpr uint8_t SDA_PIN = 5;
constexpr uint8_t SCL_PIN = 6;
constexpr int8_t SCREEN_WIDTH = 72;
constexpr int8_t SCREEN_HEIGHT = 40;

U8G2_SSD1306_72X40_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

void setup()
{
    Wire.begin(SDA_PIN, SCL_PIN);
    display.begin();
}
```

Ein einfaches Statusbild:

```cpp
void drawCenteredMessage(const char *message)
{
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tr);
    display.setCursor(0, 24);
    display.print(message);
    display.sendBuffer();
}
```

Eine zweizeilige Anzeige für den kleinen Bildschirm:

```cpp
void drawTwoLineMessage(const char *firstLine, const char *secondLine)
{
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tr);
    display.setCursor(0, 16);
    display.print(firstLine);
    display.setCursor(0, 30);
    display.print(secondLine);
    display.sendBuffer();
}
```

Ein kompaktes Statuslayout:

```cpp
void drawStatus(bool isPresent)
{
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tr);

    if (isPresent)
    {
        display.setCursor(9, 24);
        display.print("Anwesend");
    }
    else
    {
        display.setCursor(18, 16);
        display.print("Nicht");
        display.setCursor(12, 30);
        display.print("Anwesend");
    }

    display.sendBuffer();
}
```

Praktische Regeln für dieses Display:

- Kurze Wörter bevorzugen, weil 72 x 40 Pixel wenig Platz bieten.
- `u8g2_font_6x10_tr` passt gut zu ein- und zweizeiligen Statusmeldungen.
- Erst `clearBuffer()`, dann zeichnen, dann `sendBuffer()` aufrufen.
- Bei Fehlern sind zweizeilige Meldungen wie `WLAN` / `Fehler` lesbarer als lange Sätze.
- Während längerer Wartezeiten sollte die Anzeige einen klaren Betriebszustand zeigen, z. B. `Start...`, `WLAN...`, `Zeit...`, `Sende...` oder `HTTP Fehler`.

## Button-Nutzung

Der Board-Button hängt im Beispiel an GPIO9 und wird mit internem Pull-up betrieben:

```cpp
constexpr uint8_t BUTTON_PIN = 9;
constexpr uint8_t BUTTON_PRESSED_LEVEL = LOW;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 50;

void setup()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

bool readButtonPressed()
{
    return digitalRead(BUTTON_PIN) == BUTTON_PRESSED_LEVEL;
}
```

Wichtig: Bei `INPUT_PULLUP` liest der Eingang ohne Tastendruck `HIGH`. Beim Drücken wird der Pin nach GND gezogen und liest `LOW`. Die Logik wirkt deshalb zunächst invertiert.

Ein einfaches Entprellmuster:

```cpp
bool lastRawButtonPressed = false;
bool stableButtonPressed = false;
unsigned long lastButtonChangeMs = 0;

void updateButton(unsigned long nowMs)
{
    const bool rawButtonPressed = readButtonPressed();

    if (rawButtonPressed != lastRawButtonPressed)
    {
        lastRawButtonPressed = rawButtonPressed;
        lastButtonChangeMs = nowMs;
    }

    if (nowMs - lastButtonChangeMs >= BUTTON_DEBOUNCE_MS
        && rawButtonPressed != stableButtonPressed)
    {
        stableButtonPressed = rawButtonPressed;
        Serial.println(stableButtonPressed ? "Button gedrückt" : "Button losgelassen");
    }
}
```

Wenn ein zusätzlicher Sensor an GPIO4 genutzt wird, kann der Button mit dem Sensoreingang kombiniert werden. Der Zustand ist dann aktiv, wenn Sensor oder Button aktiv sind.

```cpp
bool combineInputState(bool sensorActive, bool buttonPressed)
{
    return sensorActive || buttonPressed;
}
```

## Serial-Ausgaben über USB-C

Eine wichtige Besonderheit des ESP32-C3: `Serial.print()` erscheint nicht auf jedem Board automatisch über die USB-C-Schnittstelle. Für dieses OLED-Entwicklungsboard wurden diese PlatformIO-Build-Flags benötigt:

```ini
build_flags =
    -D ARDUINO_USB_MODE=1
    -D ARDUINO_USB_CDC_ON_BOOT=1
```

Ohne `ARDUINO_USB_CDC_ON_BOOT=1` kann die Firmware laufen und das Display korrekt anzeigen, während der Serial Monitor leer bleibt.

Empfohlenes Setup im Code:

```cpp
void setup()
{
    Serial.begin(115200);
    delay(200);

    Serial.println();
    Serial.println("ESP32-C3 Firmware gestartet");
    Serial.print("Builddatum: ");
    Serial.println(__DATE__);
    Serial.print("Buildzeit: ");
    Serial.println(__TIME__);
}
```

Nach dem Flashen verbindet Windows den COM-Port manchmal neu. Wenn der Monitor leer bleibt, helfen oft: Serial Monitor schließen und neu öffnen, Reset-Taste drücken oder prüfen, ob wirklich der richtige COM-Port verwendet wird.

## WLAN- und HTTP-Hinweise

Für ESP32-Projekte mit WLAN-Anbindung gehören SSID, Passwort, Zieladresse und Zeitlimits in eine eigene Konfigurationsdatei. Lokale Zugangsdaten sollten nicht direkt in wiederverwendbare Beispielprojekte übernommen werden.

Wichtig: Auf dem ESP32 darf nicht `localhost` als Zieladresse für einen Dienst auf einem anderen Rechner verwendet werden. `localhost` zeigt aus Sicht des ESP32 auf den ESP32 selbst. Verwende stattdessen die LAN-IP oder den lokalen Hostnamen des Zielgeräts.

Ein minimales HTTP-POST-Muster:

```cpp
#include <HTTPClient.h>
#include <WiFi.h>

constexpr unsigned long HTTP_TIMEOUT_MS = 5000;
constexpr char STATUS_URL[] = "http://geraet-im-lan.local/api/status";

bool sendStatus(bool isActive)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return false;
    }

    const char *payload = isActive
        ? "{\"active\":true}"
        : "{\"active\":false}";

    HTTPClient http;
    http.setTimeout(HTTP_TIMEOUT_MS);

    if (!http.begin(STATUS_URL))
    {
        return false;
    }

    http.addHeader("Content-Type", "application/json");
    const int statusCode = http.POST(reinterpret_cast<const uint8_t *>(payload), strlen(payload));
    http.end();

    return statusCode >= 200 && statusCode < 300;
}
```

Wenn Zeitstempel übertragen werden sollen, sollte die Uhr vor dem ersten Senden per NTP synchronisiert werden:

```cpp
#include <time.h>

constexpr char NTP_SERVER[] = "pool.ntp.org";
constexpr long GMT_OFFSET_SECONDS = 0;
constexpr int DAYLIGHT_OFFSET_SECONDS = 0;

void configureUtcTime()
{
    configTime(GMT_OFFSET_SECONDS, DAYLIGHT_OFFSET_SECONDS, NTP_SERVER);
}
```

Für reine lokale Anzeigen ist NTP nicht nötig.

## Status-LED und Fehlerzustände

Im Beispielaufbau liegt die Status-LED auf GPIO8 und ist aktiv bei `LOW`:

```cpp
constexpr uint8_t STATUS_LED_PIN = 8;
constexpr uint8_t STATUS_LED_ACTIVE_LEVEL = LOW;

void setStatusLed(bool isOn)
{
    digitalWrite(STATUS_LED_PIN, isOn ? STATUS_LED_ACTIVE_LEVEL : !STATUS_LED_ACTIVE_LEVEL);
}
```

Für robuste Geräte ist eine sichtbare Fehleranzeige sinnvoll. Bei WLAN-, Zeit- oder HTTP-Fehlern kann die LED blinken, während das Display eine kurze Fehlermeldung zeigt.

## Bewährte Startreihenfolge

Diese Reihenfolge hat sich für das Board bewährt:

1. `Serial.begin(115200)` starten.
2. GPIOs für Button, Sensor und LED setzen.
3. I2C mit `Wire.begin(SDA_PIN, SCL_PIN)` starten.
4. U8g2 mit `display.begin()` starten.
5. Firmware-Version, Builddatum und Buildzeit auf Serial und OLED anzeigen.
6. WLAN verbinden.
7. Optional NTP-Zeit synchronisieren.
8. Initialen Status senden oder lokalen Startzustand anzeigen.
9. Im `loop()` Eingänge entprellen, Statuswechsel senden und Fehlerzustände sichtbar halten.

## Konfigurationsdatei

Lokale Werte sollten in `include/Config.h` liegen. Für wiederverwendbare Projekte ist ein `Config.example.h` sinnvoll, damit WLAN-Daten und lokale Zieladressen nicht versehentlich übernommen werden.

Beispiel:

```cpp
#pragma once

#include <Arduino.h>

constexpr char WIFI_SSID[] = "WLAN_SSID";
constexpr char WIFI_PASSWORD[] = "WLAN_PASSWORT";
constexpr char TARGET_STATUS_URL[] = "http://geraet-im-lan.local/api/status";

constexpr uint8_t SDA_PIN = 5;
constexpr uint8_t SCL_PIN = 6;
constexpr uint8_t BUTTON_PIN = 9;
constexpr uint8_t BUTTON_PRESSED_LEVEL = LOW;
```

## Häufige Fehlerquellen

- Falscher COM-Port in `platformio.ini` oder im Monitor-Befehl.
- Serial Monitor noch geöffnet, während geflasht werden soll.
- Fehlende USB-CDC-Build-Flags, wodurch `Serial.print()` unsichtbar bleibt.
- `localhost` als Serveradresse im ESP32-Code.
- Zu lange Texte für das 72 x 40 OLED.
- Buttonlogik falsch herum interpretiert, weil `INPUT_PULLUP` gedrückt als `LOW` liest.
- Zielgerät im WLAN nicht erreichbar, weil falsche IP-Adresse, falscher Hostname oder ein nur lokal gebundener Dienst verwendet wird.



