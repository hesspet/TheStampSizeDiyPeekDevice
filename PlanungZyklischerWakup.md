---
Datum: 26.05.2026
Version: 1
Autor: Peter Heß, Germany (+Codex)
---

# Planung Zyklischer Wakeup

## Zweck

Dieses Dokument dient als Entscheidungsvorlage und als Kontext für eine spätere Implementierung eines manuellen zyklischen Wakeup-Modus im Unterprojekt `BlePrompter`.

Ziel ist, den Stromverbrauch des ESP32-C3-OLED-Geräts in Phasen ohne unmittelbare Nutzung deutlich zu reduzieren, ohne das Gerät dauerhaft unauffindbar zu machen.

## Kernaussage

Ein Smartphone kann einen ESP32-C3 im echten DeepSleep nicht direkt per BLE wecken, weil BLE im DeepSleep abgeschaltet ist. Ein Smartphone-Wecksignal per Bluetooth ist deshalb nicht realistisch, solange das Gerät wirklich im DeepSleep ist.

Das praktikable Verfahren ist ein manuell aktivierter zyklischer DeepSleep:

- Das Gerät schläft per Timer.
- Es wacht regelmäßig kurz auf.
- Es startet in diesem Zeitfenster BLE-Advertising mit dem Nordic UART Service.
- Der Smartphone-Client kann es in diesem Zeitfenster finden und verbinden.
- Nach erfolgreicher Verbindung bleibt das Gerät aktiv.
- Sleep wird ausschließlich durch Button oder Befehl aktiviert.

Wichtig: Es darf keinen automatischen Sleep aus normaler Nutzung heraus geben. Kein automatischer Display-Sleep, kein automatischer DeepSleep nach Inaktivität.

## Anforderungen

- Sleep-Modi werden nur durch Anwenderaktion aktiviert:
  - langer Buttondruck,
  - BLE-Befehl.
- Kein automatischer Display-Sleep.
- Kein automatischer DeepSleep nach Inaktivität.
- Das Gerät muss auch dann wieder auffindbar sein, wenn kein Zugriff auf Reset, EN oder GPIO9 möglich ist.
- Eine Latenz bis ungefähr 30 Sekunden ist akzeptabel.
- Android und iOS sollen unterstützt werden.
- Der Smartphone-Client wird mit Expo als vollständiger Development Build oder EAS Build umgesetzt, nicht mit Expo Go. Dies wird in einem gesonderten Projekt durchgeführt und wird nicht in diesem Auftrag ausgeführt.

## Empfohlenes Firmware-Verfahren

Neuer Befehl:

```text
SLEEP CYCLE <sleepSeconds> <listenSeconds>
```

Beispiel:

```text
SLEEP CYCLE 25 10
```

Ablauf:

1. Das Gerät bestätigt den Befehl über die TX-Characteristic.
2. OLED und I2C werden abgeschaltet.
3. Der ESP32 geht für `sleepSeconds` in DeepSleep.
4. Nach Timer-Wakeup startet die Firmware BLE-Advertising.
5. Das Gerät bleibt für `listenSeconds` erreichbar.
6. Wenn kein Client verbindet und kein gültiger Befehl eingeht, beginnt der nächste DeepSleep-Zyklus.
7. Wenn ein Client verbindet oder ein gültiger Befehl eingeht, wird der Cycle-Modus beendet.
8. Das Gerät bleibt aktiv, bis der Anwender wieder einen Sleep-Befehl sendet oder den Sleep-Button nutzt.

## Sicherheitsgrenzen

Damit das Gerät ohne physischen Zugriff auffindbar bleibt, darf der Cycle-Modus keine beliebigen Extremwerte zulassen.

Empfohlene Grenzen:

- `listenSeconds` mindestens `10`.
- `sleepSeconds` maximal `30`, solange keine bessere Messbasis vorliegt.
- Ungültige Werte werden abgelehnt oder auf sichere Grenzen geklemmt.
- Nach mehreren kurzen Zyklen kann ein längeres Suchfenster folgen, zum Beispiel nach jedem 10. Zyklus `60 s` Advertising.

Das längere Suchfenster reduziert das Risiko, dass iOS oder Android mehrere kurze Advertising-Fenster verpasst.

## Aktiv Bleiben

Zusätzlicher Befehl:

```text
WAKE
```

Der Befehl `WAKE` beendet den Cycle-Modus explizit und hält das Gerät aktiv.

Begründung:

- kurz,
- klar,
- makrotauglich,
- unabhängig von der bestehenden `SLEEP`-Befehlsgruppe.

Alternativ wäre `SLEEP STAY` möglich, aber `WAKE` ist für Smartphone-Makros einfacher.

## Gerätekennung

Der BLE-Name allein ist keine Identität. Zwei Geräte können denselben Namen `BlePrompter` haben.

Deshalb soll die Firmware eine kurze eindeutige Gerätekennung aus der ESP32-Chip-ID ableiten, zum Beispiel:

```text
BP-3F8A
```

Empfohlene Darstellung im BLE-Namen oder in der Scan Response:

```text
BlePrompter-3F8A
```

Diese Kennung soll:

- im Advertising-Namen oder in der Scan Response sichtbar sein,

- auf dem Start- oder Bereitschaftsbildschirm erscheinen,

- die Information soll auch auf Serial herausgeschrieben werden

  Beispiel:

```text
Name: BlePrompter-3F8A
Id: BP-3F8A
Version: 1.x.x
Service: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
```

## Smartphone-Verfahren

Die App bekommt eine Einstellungsseite `Gerät auswählen`.

Erstauswahl:

1. Die App scannt nach der Nordic UART Service UUID.
2. Die App zeigt alle gefundenen Geräte an.
3. Bei mehreren Geräten zeigt sie nicht nur den Namen, sondern auch:
   - Gerätekennung, zum Beispiel `BP-3F8A`,
   - Signalstärke/RSSI,
   - zuletzt gesehen,
   - Plattform-ID aus `device.id`.
4. Der Nutzer wählt das gewünschte Gerät.
5. Die App speichert:
   - `device.id`,
   - Gerätekennung,
   - Gerätename,
   - Service UUID,
   - RX-Characteristic UUID,
   - TX-Characteristic UUID,
   - Datum der letzten erfolgreichen Verbindung im Format `DD.MM.YYYY`.

Wiederverbindung:

1. Die App versucht zuerst, das gespeicherte Gerät per `device.id` zu verwenden.
2. Wenn das fehlschlägt, scannt sie nach der Nordic UART Service UUID.
3. Wenn die gespeicherte Gerätekennung wieder auftaucht, verbindet sie erneut.
4. Falls mehrere Geräte gefunden werden und keine eindeutige Zuordnung möglich ist, muss der Nutzer neu auswählen.

## Suchmodus Für Schlafende Geräte

Die App bietet einen Modus `Schlafendes Gerät suchen`.

Dieser Modus:

- scannt länger, zum Beispiel `90-120 s`,
- wartet auf eines der zyklischen Advertising-Fenster,
- verbindet sofort, sobald das Zielgerät sichtbar wird,
- sendet nach erfolgreicher Verbindung `WAKE`,
- aktualisiert die gespeicherten Gerätedaten,
- öffnet danach die normale Bedienoberfläche.

Damit ist kein separater Pairing-Modus per Button nötig. Der Cycle-Modus selbst ist der langsame Auffindmodus.

## iOS Und Expo

Unter iOS gibt es für Apps keine BLE-MAC-Adresse. `react-native-ble-plx` liefert unter Android typischerweise eine MAC-Adresse als `device.id`, unter iOS aber eine UUID.

Für diesen Anwendungsfall ist das ausreichend, solange die App die iOS-UUID als app-lokale Geräteidentität behandelt und zusätzlich die vom ESP32 gelieferte Gerätekennung speichert.

Machbarer Expo-Pfad:

- Expo Development Build oder EAS Build verwenden.
- Nicht Expo Go verwenden.
- `react-native-ble-plx` mit Config Plugin einbinden.
- iOS Bluetooth-Permission lokalisieren.
- Optional den iOS-Hintergrundmodus `bluetooth-central` aktivieren.
- `BleManager` als Singleton initialisieren.
- Optional `restoreStateIdentifier` und `restoreStateFunction` nutzen.

Einschränkungen:

- Die iOS-UUID ist keine öffentliche MAC-Adresse.
- Die iOS-UUID ist nicht sinnvoll zwischen verschiedenen iPhones übertragbar.
- Nach App-Neuinstallation oder iOS-seitigem Verlust der Zuordnung kann eine neue Geräteauswahl nötig sein.
- Hintergrund-BLE unter iOS ist möglich, aber vom System begrenzt und muss auf echtem Gerät getestet werden.

## Risiken

- Zu kurze Advertising-Fenster können von iOS oder Android verpasst werden.
- Zu lange Sleep-Fenster machen das Gerät praktisch schwer auffindbar.
- Mehrere Geräte mit gleichem Namen erfordern eine sichtbare Gerätekennung.
- iOS-Hintergrund-BLE kann je nach Systemzustand verzögert arbeiten.
- Wenn der Cycle-Modus falsch konfiguriert wird, kann das Gerät ohne physischen Zugriff schwer erreichbar werden.

## Testfälle Firmware

- `SLEEP CYCLE 25 10` aktiviert zyklischen DeepSleep.
- Gerät advertised in jedem Wake-Fenster.
- Ohne Client schläft das Gerät nach dem Listen-Fenster wieder ein.
- Bei Verbindung wird der Cycle-Modus beendet.
- Bei gültigem Befehl wird der Cycle-Modus beendet.
- `WAKE` hält das Gerät aktiv.
- Bestehende Befehle `SLEEP DISPLAY`, `SLEEP DEEP <Sekunden>` und `SLEEP RESET` bleiben funktionsfähig.
- Ungültige Cycle-Werte werden abgelehnt oder auf sichere Grenzen geklemmt.
- Nach mehreren kurzen Zyklen erscheint das längere Suchfenster.

## Testfälle Android

- Gerät auswählen und `device.id` speichern.
- Gerät in Cycle-Modus versetzen.
- Schlafendes Gerät innerhalb von `90-120 s` wiederfinden.
- Verbindung herstellen und `WAKE` senden.
- Mehrere `BlePrompter-*` Geräte eindeutig unterscheiden.

## Testfälle iOS

- Expo Development Build auf echtem iPhone testen.
- Erstscan nach Nordic UART Service durchführen.
- `device.id` speichern.
- App neu starten und Gerät wiederfinden.
- Cycle-Modus über langen Suchlauf finden.
- Verbindung herstellen und `WAKE` senden.
- Mehrere `BlePrompter-*` Geräte eindeutig unterscheiden.
- Optional Hintergrundverhalten mit `bluetooth-central` und State Restoration testen.

## Quellen Und Prüfhinweise

- Expo Development Builds und native Konfiguration: `https://docs.expo.dev/develop/development-builds/introduction/`
- Expo Continuous Native Generation: `https://docs.expo.dev/workflow/continuous-native-generation/`
- `react-native-ble-plx`: `https://dotintent.github.io/react-native-ble-plx/`
- `react-native-ble-plx` Expo-Hinweise: `https://github.com/dotintent/react-native-ble-plx/wiki/Expo`
- `react-native-ble-plx` iOS Background Mode: `https://github.com/dotintent/react-native-ble-plx/wiki/Background-mode-(iOS)`
- Apple Core Bluetooth Background Processing: `https://developer.apple.com/library/archive/documentation/NetworkingInternetWeb/Conceptual/CoreBluetooth_concepts/CoreBluetoothBackgroundProcessingForIOSApps/PerformingTasksWhileYourAppIsInTheBackground.html`

## Entscheidungsempfehlung

Der manuell aktivierte zyklische DeepSleep ist der beste Kompromiss aus Stromersparnis und Wiederauffindbarkeit.

Die empfohlene Umsetzung ist:

- Firmware-Befehl `SLEEP CYCLE <sleepSeconds> <listenSeconds>`.
- Firmware-Befehl `WAKE`.
- Kurze eindeutige Gerätekennung wie `BP-3F8A`.
- BLE-Name oder Scan Response mit sichtbarer Kennung.
- Smartphone-App mit gespeicherter `device.id` und gespeicherter Gerätekennung.
- Langer Suchmodus für schlafende Geräte über `90-120 s`.
- Keine automatische Sleep-Aktivierung aus normalem Betrieb.
