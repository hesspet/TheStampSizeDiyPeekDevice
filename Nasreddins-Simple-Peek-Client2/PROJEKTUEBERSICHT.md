# Projektübersicht: Nasreddins Simple Peek Client 2

Stand: 25.05.2026

## Zweck

Dieses Projekt ist die lokale Entwicklungsbasis für die Expo-/React-Native-App `Nasreddins Simple Peek Client 2`. Die App ist ein Client für das Projekt `C:\dev\TheStampSizeDiyPeekDevice\BlePrompter` und kommuniziert per Bluetooth Low Energy mit der dortigen BlePrompter-Hardware. Weil dafür native BLE-Bibliotheken gebraucht werden, wird nicht mit Expo Go gearbeitet, sondern mit einem Expo Development Build über `expo-dev-client`.

## Projektpfade

- Projektordner: `C:\dev\Nasreddins-Simple-Peek-Client2`
- Projektpaketname in `package.json`: `nasreddins-simple-peek-client2`
- App-Name: `Nasreddins Simple Peek Client 2`
- Android Application ID: `de.nasreddin.simplepeekclient2`
- Android-Projektordner: `C:\dev\Nasreddins-Simple-Peek-Client2\android`
- Debug-APK: `C:\dev\Nasreddins-Simple-Peek-Client2\android\app\build\outputs\apk\debug\app-debug.apk`
- Expo-Dev-Server-Log: `C:\dev\Nasreddins-Simple-Peek-Client2\.expo-start.out.log`

## Aktueller technischer Stand

- Expo-Projekt wurde mit `create-expo-app` und Template `default@sdk-56` erzeugt.
- Installierte Kernversionen:
  - Node.js: `v24.16.0`
  - npm: `11.13.0`
  - Expo: `~56.0.3`
  - Expo Dev Client: `~56.0.14`
  - React Native: `0.85.3`
- BLE-Bibliothek: `react-native-ble-plx` `^3.5.1`
- Kamera-Bibliothek: `expo-camera` `~56.0.7`
- `expo-dev-client` ist installiert.
- Native Android-Dateien wurden durch Expo Prebuild erzeugt.
- Die Expo-Starteroberfläche wurde durch eine native BlePrompter-Oberfläche ersetzt.
- Die App enthält ein Hamburger-Menü mit `Peeker (Videofake)`, `Einstellungen`, `Log` und `About`.
- Im Kopfbereich wird statt `BLE-UART` eine Expo-Metadatenzeile mit App-Version, Buildnummer und letztem Update-Datum angezeigt. Das Update-Datum kommt aus dem Expo-Update-Manifest, wenn die Laufzeit es bereitstellt; sonst erscheint `Update unbekannt`.
- Die Ansichten `Pfeile`, `Karten`, `Symbole`, `Raw` und `ESP` öffnen nach Antippen als LiveVideo-Vollbildansicht mit echter Kamera-Vorschau, Aufnahme-HUD und zunächst ausgeblendeten Bedienfeldern. Diese fünf Overlays heißen in Aufträgen kurz `Kamera-Buttonansichten`. Ein Tippen auf das Kamerabild blendet die Bedienfelder als Overlay ein; `Bedienung ausblenden`, `Zurück` und die Android-Hardware-Zurück-Taste steuern Overlay und Rücknavigation.
- Die Ansicht `Raw` ist ein Low-Level-Sendekanal mit freier einzeiliger Eingabe. Der dort erfasste Befehl wird unverändert ohne Prefix, Kürzung oder Großschreibung an BlePrompter gesendet.
- Die LiveVideo-Vollbildansicht unterstützt Pinch-to-Zoom auf dem Kamerabild, zeigt den Zoomwert unten an und enthält einen rein optischen Pause-Button.
- In den Einstellungen kann die Kameraansicht manuell zwischen `Android`, `iOS` und `Fantasie` umgeschaltet werden. Es gibt keine automatische Plattformauswahl.
- In den Einstellungen kann die Darstellung der Kamera-Bedienung zwischen `Normal`, `Durchsichtig`, `Unauffällig` und `Anpassbar` umgeschaltet werden. `Anpassbar` blendet einen React-Native-Bordmittel-Slider für die Deckkraft ein, damit die Buttons je nach Showsituation sichtbarer oder fast unsichtbar wirken können.
- In den Einstellungen gibt es die persistenten Display-Optionen `Displayanzeige invertiert` und `Displayanzeige drehen`. Bei aktiver BLE-Verbindung werden Änderungen sofort als `I0`/`I1` beziehungsweise `U0`/`U1` an das Gerät gesendet.
- Nach einer erfolgreichen BLE-Verbindung merkt sich die App das zuletzt genutzte Gerät. Beim nächsten Appstart wird ein Popup angezeigt und die App versucht automatisch, dieses Gerät wiederzufinden und zu verbinden. Der Anwender kann den Verbindungsversuch im Popup abbrechen. Wenn das Gerät nicht gefunden wird, bleibt die App im normalen unverbundenen Modus und der Anwender verbindet manuell neu.
- Direkt nach jedem erfolgreichen Verbindungsaufbau synchronisiert die App die Display-Optionen immer in fester Reihenfolge: zuerst `I0`/`I1` für invertierte Anzeige, danach `U0`/`U1` für gedrehte Anzeige. Das gilt auch für den automatischen Reconnect.
- Der Anwender-Befehl `Anzeige löschen` sendet zuerst `CL` und nach erfolgreichem Senden direkt `SLEEP DISPLAY`, damit das OLED nach dem Löschen in den Display-Sleep geht und BLE aktiv bleibt.
- Die App ist nicht mehr auf Hochformat gesperrt. `app.json` verwendet `orientation: default`, und die Vollbild-Bedienfelder passen ihre Grids für Hoch- und Querformat an.
- Die App hat einen Testmodus, in dem keine Bluetooth-Berechtigungen angefragt und keine BLE-Funktionen benutzt werden. Kommandos werden nur im Log simuliert, damit die Oberfläche im Android-Emulator getestet werden kann.
- Laufende Logausgaben werden im eigenen Menüpunkt `Log` angezeigt und nicht mehr auf dem Client-Hauptscreen.
- Der About-Screen lädt seinen Inhalt aus `about.md` im Projektwurzelordner. Das Nasreddin-Bild wird dort als kleines transparentes Icon oberhalb des Markdown-Inhalts angezeigt.
- Die App-Icons wurden aus `NasreddinDerMagier.png` erzeugt: Figur im weißen Kreis auf türkisfarbenem Hintergrund. Die Expo-Starterbilddateien wurden entfernt.
- Erster lokaler Android Development Build wurde erfolgreich gebaut.
- Das Debug-APK wurde erfolgreich auf dem Emulator installiert.
- Das Debug-APK wurde erfolgreich auf dem Xiaomi-Smartphone `YTXOFMS8EM5LTOBA` installiert.
- Die App wurde erfolgreich über den Expo Development Client gestartet.
- Die Einstellungen werden mit `@react-native-async-storage/async-storage` lokal persistiert und beim Neustart wieder geladen.
- Metro/Expo hat das JavaScript-Bundle erfolgreich gebaut.

## Development-Build-Verhalten

Die aktuell gebaute APK ist ein Expo Development Build mit `expo-dev-client`. Beim Start erscheint deshalb zuerst der Expo Dev Client. Dort wird eine Verbindung zum Metro-/Expo-Server auf dem Entwicklungsrechner aufgebaut. Erst danach lädt die App den aktuellen JavaScript-/TypeScript-Code.

Das ist für die Entwicklung korrekt:

- Die APK enthält die native Android-Hülle und native Module wie `react-native-ble-plx`.
- Metro liefert die aktuelle React-Native-Oberfläche.
- Expo Go wird nicht verwendet und wäre für diese App ungeeignet, weil Expo Go `react-native-ble-plx` nicht enthält.
- Ohne laufenden Metro-Server startet die Development-Build-App nicht direkt in die BlePrompter-Oberfläche.

Für eine spätere Nutzerversion muss ein Standalone-, Preview- oder Release-Build erstellt werden, bei dem das JavaScript in die App gebündelt wird. Dann verschwindet die Expo-Verbindungsseite und die App startet direkt als BlePrompter-App.

## Lokales Android-Environment

- Android Studio ist installiert unter `C:\Program Files\Android\Android Studio`.
- Android SDK ist installiert unter `C:\Users\hesspet\AppData\Local\Android\Sdk`.
- Benutzer-Umgebungsvariablen:
  - `ANDROID_HOME=C:\Users\hesspet\AppData\Local\Android\Sdk`
  - `ANDROID_SDK_ROOT=C:\Users\hesspet\AppData\Local\Android\Sdk`
  - `JAVA_HOME=C:\Program Files\Android\Android Studio\jbr`
- Android SDK Command-line Tools sind installiert:
  - `cmdline-tools;20.0`
  - `platform-tools;37.0.0`
  - `emulator;36.5.11`
  - `platforms;android-36.1`
  - `build-tools;37.0.0`
  - `system-images;android-36.1;google_apis_playstore;x86_64`
- SDK-Lizenzen wurden akzeptiert.

## Geräte und Emulator

- Angelegter Emulator: `Expo_Pixel_7_API_36_1`
- Laufende ADB-Seriennummer des Emulators, wenn gestartet: `emulator-5554`
- Angeschlossenes Android-Smartphone:
  - Seriennummer: `YTXOFMS8EM5LTOBA`
  - Hersteller: `Xiaomi`
  - Modell: `23078PND5G`
  - Android-Version: `16`
  - API-Level: `36`
- Aktueller Stand beim letzten Test:
  - Emulator war gestartet und in `adb devices` sichtbar.
  - Smartphone war ebenfalls in `adb devices` sichtbar.

## Wichtige technische Entscheidungen

- Schwerpunkt ist zunächst lokale Android-Entwicklung.
- iOS wird später über EAS Build auf Expo-Servern gebaut und danach über EAS Submit beziehungsweise Apple-Werkzeuge verteilt.
- Expo Go wird nicht verwendet.
- BLE-Bibliothek ist `react-native-ble-plx`.
- `react-native-ble-plx` ist in `app.json` als Config Plugin mit `isBackgroundEnabled: false` und `neverForLocation: true` konfiguriert.
- BLE sollte realistisch zuerst auf dem echten Android-Gerät getestet werden, weil Emulatoren BLE-Hardwarezugriff nur eingeschränkt oder gar nicht sinnvoll abbilden.
- Hinweis für iOS: Der automatische Reconnect über das zuletzt gespeicherte BLE-Gerät ist vorerst nicht gesondert behandelt. iOS kann bei BLE-Identifiern und Wiederverbindungslogik andere Einschränkungen haben als Android; das muss später auf echter iOS-Hardware geprüft werden.
- Für reine Oberflächentests im Emulator gibt es den Testmodus. Er deaktiviert Bluetooth vollständig und simuliert Befehle nur im Log.
- User-facing Strings müssen lokalisiert werden und deutsche Umlaute verwenden.
- Datumsformate verwenden `DD.MM.YYYY`.
- Abkürzungen in Variablen- und Methodennamen sollen vermieden werden.

## Bekannte Anpassungen und Probleme

- Der von Expo erzeugte Gradle Wrapper wurde von `9.3.1` auf `8.14.3` geändert.
  - Grund: Gradle 9 scheiterte beim Build mit `Class org.gradle.jvm.toolchain.JvmVendorSpec does not have member field 'org.gradle.jvm.toolchain.JvmVendorSpec IBM_SEMERU'`.
  - Die Änderung steht in `android\gradle\wrapper\gradle-wrapper.properties`.
- Der direkte `npx expo run:android --device ...`-Weg war in dieser Umgebung nicht stabil genug, weil Geräteauswahl, Build, Installation und Start gekoppelt sind.
- Der erfolgreiche Weg war bewusst getrennt:
  1. APK direkt mit Gradle bauen.
  2. Emulator separat starten.
  3. APK per `adb install` installieren.
  4. Expo-Dev-Server separat starten.
  5. Dev Client per Deep Link öffnen.
- `npm install` meldete zuletzt keine bekannten Vulnerabilities.
- Es gibt alte AVD-Einträge mit fehlenden System-Images im Benutzerprofil. Sie wurden nicht verändert.
- PowerShell 5 kann UTF-8-Dateien ohne BOM in der Konsole manchmal falsch anzeigen. Die Projektdateien selbst sind UTF-8.
- Beim Starten von Metro über PowerShell können lokale `fnm`-/Profilprobleme auftreten. Wenn `Start-Process` an doppeltem `Path`/`PATH` scheitert, Metro direkt in einem sichtbaren Terminal mit `npx expo start --dev-client --lan --port 8081` starten.

## Lokale Toolsammlung

Im Ordner `tools` liegen anklickbare Windows-Batchdateien für die wichtigsten Arbeitsschritte ohne Codex:

- `Build Development Version.cmd` baut die Android Development APK.
- `Starte Development Server.cmd` startet Expo/Metro auf Port `8081`.
- `Install Update via ADB.cmd` installiert die vorhandene APK per ADB, setzt die Portweiterleitung und öffnet den Expo Development Client.

Die Batchdateien rufen PowerShell-Skripte auf. Gemeinsame Pfade und Umgebungsvariablen stehen in `tools\Gemeinsame-Umgebung.ps1`.

## Erfolgreiche Build- und Startbefehle

Für neue Terminals zuerst in das Projekt wechseln:

```powershell
cd C:\dev\Nasreddins-Simple-Peek-Client2
```

Umgebungsvariablen für die aktuelle Shell setzen:

```powershell
$env:ANDROID_HOME='C:\Users\hesspet\AppData\Local\Android\Sdk'
$env:ANDROID_SDK_ROOT=$env:ANDROID_HOME
$env:JAVA_HOME='C:\Program Files\Android\Android Studio\jbr'
$env:Path="$env:ANDROID_HOME\platform-tools;$env:ANDROID_HOME\emulator;$env:ANDROID_HOME\cmdline-tools\latest\bin;$env:JAVA_HOME\bin;$env:Path"
```

Debug-APK direkt bauen:

```powershell
cd C:\dev\Nasreddins-Simple-Peek-Client2\android
.\gradlew.bat app:assembleDebug -x lint -x test --configure-on-demand --build-cache -PreactNativeArchitectures=x86_64 --console=plain --warning-mode=summary
```

Emulator starten:

```powershell
Start-Process -FilePath "$env:ANDROID_HOME\emulator\emulator.exe" -ArgumentList @('-avd','Expo_Pixel_7_API_36_1')
```

Bootstatus prüfen:

```powershell
adb devices
adb -s emulator-5554 shell getprop sys.boot_completed
```

APK installieren:

```powershell
adb -s emulator-5554 install -r C:\dev\Nasreddins-Simple-Peek-Client2\android\app\build\outputs\apk\debug\app-debug.apk
```

Expo-Dev-Server starten:

```powershell
cd C:\dev\Nasreddins-Simple-Peek-Client2
npx expo start --dev-client --lan --port 8081
```

ADB-Portweiterleitung setzen:

```powershell
adb -s emulator-5554 reverse tcp:8081 tcp:8081
```

Development Client im Emulator öffnen:

```powershell
adb -s emulator-5554 shell am start -W -a android.intent.action.VIEW -d "exp+nasreddins-simple-peek-client2://expo-development-client/?url=http%3A%2F%2F127.0.0.1%3A8081" de.nasreddin.simplepeekclient2
```

Prüfen, ob die App im Vordergrund läuft:

```powershell
adb -s emulator-5554 shell dumpsys window | Select-String -Pattern "mCurrentFocus|mFocusedApp"
```

Erwartetes Ergebnis:

```text
de.nasreddin.simplepeekclient2/.MainActivity
```

APK auf das Xiaomi-Smartphone installieren:

```powershell
adb -s YTXOFMS8EM5LTOBA install -r C:\dev\Nasreddins-Simple-Peek-Client2\android\app\build\outputs\apk\debug\app-debug.apk
```

Wenn `INSTALL_FAILED_USER_RESTRICTED` erscheint, wurde die Installation auf dem Smartphone abgelehnt. Dann am Gerät den MIUI-/Android-Dialog bestätigen beziehungsweise `Über USB installieren` erlauben und denselben Befehl erneut ausführen.

## Merksatz für den nächsten Chat

Das Projekt ist bereits lokal Android-buildfähig und als BlePrompter-App mit `react-native-ble-plx` umgesetzt. Für Emulator-Tests zuerst den Testmodus verwenden; echte BLE-Funktion danach auf dem Xiaomi-Gerät testen.
