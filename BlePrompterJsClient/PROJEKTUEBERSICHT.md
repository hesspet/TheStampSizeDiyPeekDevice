---
Datum: 22.05.2026
Version: 4
Autor: Peter Heß, Germany (+Codex)
---

# BlePrompterJsClient

`BlePrompterJsClient` ist eine statische JavaScript-Webanwendung zur Steuerung der Firmware `BlePrompter` per Web Bluetooth.

## Zweck

Die Anwendung stellt eine kleine Bedienoberfläche für Demo, Android-Layouttest und GitHub Pages bereit. Sie spricht über BLE-UART mit dem ESP32-C3-OLED-Board und sendet Textbefehle an die RX-Characteristic des Nordic UART Service.

## Dateien

| Datei | Zweck |
| --- | --- |
| `index.html` | Einstieg der statischen Webanwendung. |
| `styles.css` | Responsives Layout für Smartphone und Desktop. |
| `src/main.js` | Web-Bluetooth-Verbindung, Befehlsversand und UI-Logik. |
| `start-demo-server.bat` | Startet einen lokalen HTTPS-Demo-Webserver auf `0.0.0.0`. |
| `scripts/create-demo-certificate.ps1` | Erzeugt eine lokale Root-CA und ein Serverzertifikat mit LAN-IP-Adressen. |
| `scripts/https-demo-server.js` | Statischer HTTPS-Server für die Demo. |
| `.nojekyll` | Deaktiviert Jekyll-Verarbeitung auf GitHub Pages. |

## BLE-Konfiguration

- Bluetooth-Name: `BlePrompter`
- Protokoll: BLE-UART kompatibel zum Nordic UART Service
- Service-UUID: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- RX-Characteristic zum Schreiben: `6e400002-b5a3-f393-e0a9-e50e24dcca9e`
- TX-Characteristic für Antworten: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`

Die Befehle werden per `TextEncoder` als UTF-8-Text geschrieben. Ein Zeilenumbruch wird nicht gesendet.

## Geräteauswahl

Der Button `Verbinden` öffnet die Web-Bluetooth-Geräteauswahl mit zwei Filtern:

- Bluetooth-Name beginnt mit `BlePrompter`.
- Advertisement enthält den Nordic-UART-Service.

Falls Chrome das Gerät damit nicht findet, kann `Alle BLE-Geräte anzeigen` verwendet werden. Dieser zweite Button nutzt `acceptAllDevices: true` und fordert den Nordic-UART-Service nur als `optionalServices` an. Das ist nützlich, wenn Windows das Gerät kennt, Chrome aber den Namen oder Service nicht passend aus den aktuellen Advertisements liest.

## Betriebsarten

### Pfeile

Die Pfeilansicht zeigt eine einfache Kompassrose mit acht Richtungen. Die Oberfläche ist deutsch beschriftet, die gesendeten Kurzbefehle bleiben kompatibel zur Firmware:

| UI | Befehl |
| --- | --- |
| `N` | `AN` |
| `NO` | `ANE` |
| `O` | `AE` |
| `SO` | `ASE` |
| `S` | `AS` |
| `SW` | `ASW` |
| `W` | `AW` |
| `NW` | `ANW` |

### Karten

Die Kartenansicht bietet zuerst die Auswahl der Farbe und danach die Kartenwerte. Die Karte `10` wird als `X` gesendet.

| UI | Befehlsteil |
| --- | --- |
| Herz | `H` |
| Karo | `D` |
| Kreuz | `C` |
| Pik | `S` |
| Ass | `1` |
| 10 | `X` |
| Bube | `J` |
| Dame | `Q` |
| König | `K` |
| Joker 1 | `CJ1` |
| Joker 2 | `CJ2` |

Beispiel: Herz 10 sendet `CHX`, Pik König sendet `CSK`.

### Symbole

Die Symbolansicht erlaubt maximal zwei Zeichen. Gesendet wird der Langbefehl:

```text
SYMBOL <Zeichen>
```

Die Firmware stellt die Zeichen groß und zentriert auf dem OLED dar.

### ESP

Die ESP-Ansicht bietet fünf Buttons für die klassischen ESP-/Zener-Symbole. Die Oberfläche ist deutsch beschriftet, die gesendeten Kurzbefehle bleiben englisch-kompakt:

| UI | Befehl |
| --- | --- |
| Kreis | `EC` |
| Kreuz | `EG` |
| Wellen | `EW` |
| Quadrat | `EQ` |
| Stern | `ES` |

## Lokaler HTTPS-Demo-Server

Der lokale Webserver wird mit folgender Datei gestartet:

```powershell
BlePrompterJsClient\start-demo-server.bat
```

Standard-Port ist `8443`. Ein anderer Port kann als Argument übergeben werden:

```powershell
BlePrompterJsClient\start-demo-server.bat 9443
```

Die Batchdatei erzeugt beim ersten Start unter `certificates/` eine lokale Root-CA und ein Serverzertifikat. Danach startet sie:

```powershell
node scripts\https-demo-server.js 8443
```

Dadurch ist die Seite im lokalen Netz erreichbar, zum Beispiel:

```text
https://<IP-Adresse des PCs>:8443/
```

Für Smartphones muss `certificates\ble-prompter-demo-root-ca.cer` als vertrauenswürdiges Zertifikat installiert werden. Ohne Vertrauen in diese lokale Root-CA kann Chrome die Seite als unsicher behandeln und Web Bluetooth blockieren.

## Web-Bluetooth-Hinweise

Web Bluetooth ist nur in sicheren Kontexten verfügbar. Für echte BLE-Verbindungen funktionieren in der Praxis:

- `https://...`, zum Beispiel GitHub Pages.
- `https://<IP-Adresse des PCs>:8443/`, wenn das lokale Root-Zertifikat auf dem Smartphone vertraut ist.
- `http://localhost:8080` oder `https://localhost:8443` auf demselben Gerät.

Der frühere Zugriff über `http://<PC-IP>:8080` ist nur für UI-Demos und Android-Layouttests geeignet. Für echte BLE-Verbindungen auf Android braucht die Seite einen gültigen sicheren Kontext.

## GitHub Pages

Das Unterprojekt ist ohne Build-Schritt nutzbar. Für GitHub Pages kann der Ordner `BlePrompterJsClient` als statischer Inhalt veröffentlicht werden. Die Datei `.nojekyll` verhindert zusätzliche Jekyll-Verarbeitung.
