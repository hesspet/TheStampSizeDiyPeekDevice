# TheStampSizeDiyPeekDevice

Projekt für kleine ESP32-C3-OLED-Boards mit 72 x 40 Pixel SSD1306-Display.

## Unterprojekte

- `BoardTest`: Test-Firmware für Display, Button, Pfeile, Symbole und Spielkarten.
- `BlePrompter`: BLE-UART-Firmware für Android-Tools, MacroDroid und Web-Bluetooth-Seiten.

## BlePrompter

`BlePrompter` meldet sich per BLE als `BlePrompter` und nutzt den Nordic UART Service.

- Service-UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- RX-Characteristic zum Schreiben: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
- TX-Characteristic für Antworten: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`

Für eine spätere JavaScript-Seite sind die relevanten Web-Bluetooth-Hinweise in `BlePrompter/WEB_BLUETOOTH.md` dokumentiert. Die BLE-Befehle stehen in `BlePrompter/BEFEHLE.md`.
