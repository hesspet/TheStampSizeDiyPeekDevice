---
Datum: 25.05.2026
Version: 3
Autor: Peter Heß, Germany (+Codex)
---

# BlePrompter Web Bluetooth

Diese Datei ist der Einstieg für eine spätere JavaScript-Seite, die im Browser per Web Bluetooth mit `BlePrompter` spricht.

## Zielgerät

- Bluetooth-Name: `BlePrompter`
- Protokoll: BLE-UART kompatibel zum Nordic UART Service
- Service-UUID: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- RX-Characteristic zum Schreiben: `6e400002-b5a3-f393-e0a9-e50e24dcca9e`
- TX-Characteristic für Antworten: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`

Die JavaScript-Seite soll Befehle als UTF-8-Text an die RX-Characteristic schreiben. Ein Zeilenumbruch ist nicht nötig.

## Browser-Hinweise

Web Bluetooth ist im Browser nur in sicheren Kontexten verfügbar. Für die Entwicklung bedeutet das praktisch: `https://...` oder `localhost`. Die Geräteauswahl muss durch eine Benutzeraktion ausgelöst werden, zum Beispiel durch einen Button-Klick.

Die Service-UUID muss bei `navigator.bluetooth.requestDevice(...)` in `optionalServices` stehen. Ohne diesen Eintrag kann der Browser das Gerät zwar auswählen, aber später nicht auf den Nordic-UART-Service zugreifen.

## Minimaler Verbindungsablauf

```js
const BLE_PROMPTER_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const BLE_PROMPTER_RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const BLE_PROMPTER_TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

let blePrompterDevice;
let blePrompterServer;
let blePrompterReceiveCharacteristic;
let blePrompterTransmitCharacteristic;

async function connectBlePrompter() {
  blePrompterDevice = await navigator.bluetooth.requestDevice({
    filters: [{ namePrefix: "BlePrompter" }],
    optionalServices: [BLE_PROMPTER_SERVICE_UUID],
  });

  blePrompterServer = await blePrompterDevice.gatt.connect();
  const service = await blePrompterServer.getPrimaryService(BLE_PROMPTER_SERVICE_UUID);
  blePrompterReceiveCharacteristic = await service.getCharacteristic(BLE_PROMPTER_RX_UUID);
  blePrompterTransmitCharacteristic = await service.getCharacteristic(BLE_PROMPTER_TX_UUID);
}

async function sendBlePrompterCommand(commandText) {
  if (!blePrompterReceiveCharacteristic) {
    throw new Error("BlePrompter is not connected.");
  }

  const commandBytes = new TextEncoder().encode(commandText);

  if (blePrompterReceiveCharacteristic.writeValueWithoutResponse) {
    await blePrompterReceiveCharacteristic.writeValueWithoutResponse(commandBytes);
    return;
  }

  await blePrompterReceiveCharacteristic.writeValueWithResponse(commandBytes);
}
```

## Optionale Antworten lesen

Die Firmware schreibt kurze Statusantworten auf die TX-Characteristic. Eine Webseite kann Notifications abonnieren.

```js
async function startBlePrompterNotifications(onMessage) {
  await blePrompterTransmitCharacteristic.startNotifications();
  blePrompterTransmitCharacteristic.addEventListener("characteristicvaluechanged", (event) => {
    const message = new TextDecoder().decode(event.target.value);
    onMessage(message);
  });
}
```

## Befehle

Die vollständige Syntax liegt in `BEFEHLE.md`. Für eine Web-Bluetooth-Seite sind diese Kurzbefehle besonders geeignet:

| Befehl | Wirkung |
| --- | --- |
| `CL` | Display löschen |
| `H` | Help senden |
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
| `ANE` | Arrow Northeast anzeigen |
| `ASW` | Arrow Southwest anzeigen |
| `CHX` | Card Heart 10 anzeigen |
| `CD7` | Card Diamond 7 anzeigen |
| `CCJ` | Card Clubs Jack anzeigen |
| `CSK` | Card Spade King anzeigen |
| `CJ1` | Joker 1 anzeigen |

Auch Langbefehle funktionieren:

```text
TEXT Door|open
SYMBOL OK
ESP Circle
ARROW NE
CARD Heart X
INVERT ON
U1
CLEAR
```

## UI-Hinweise für die spätere Webseite

- Ein sichtbarer `Connect`-Button muss die Geräteauswahl starten.
- Nach erfolgreicher Verbindung sollten Befehlsbuttons erst aktiv werden.
- Bei kopfüber montiertem Display kann die Webseite nach dem Verbindungsaufbau einmalig `U1` senden.
- Befehlsbuttons können direkt `sendBlePrompterCommand("CHX")` oder ähnliche Kommandos aufrufen.
- Für freie Texteingabe sollte die Webseite `TEXT ` voranstellen und Zeilen mit `|` trennen.
- Bei Verbindungsabbruch sollte die Webseite den verbundenen Zustand zurücksetzen und erneut `connectBlePrompter()` anbieten.

## Quellen

- MDN `Bluetooth.requestDevice()`: https://developer.mozilla.org/en-US/docs/Web/API/Bluetooth/requestDevice
- MDN `BluetoothRemoteGATTCharacteristic.writeValueWithoutResponse()`: https://developer.mozilla.org/en-US/docs/Web/API/BluetoothRemoteGATTCharacteristic/writeValueWithoutResponse
