# Nasreddins Simple Peek Client 2

Android-Entwicklungsprojekt für eine Expo-/React-Native-App zur Steuerung eines `BlePrompter` per Bluetooth Low Energy.

Die App ist ein Client für das Projekt `C:\dev\TheStampSizeDiyPeekDevice\BlePrompter`. Sie ist eine native Portierung der früheren statischen Web-App `BlePrompterJsClient`, nutzt `react-native-ble-plx` und kommuniziert über den Nordic UART Service mit der Firmware.

## Aktueller Stand

- Expo SDK `56`, React Native `0.85.3`, `expo-dev-client`.
- BLE-Bibliothek: `react-native-ble-plx` `^3.5.1`.
- Android Development Build ist erfolgreich gebaut und auf Emulator sowie Xiaomi-Smartphone installiert worden.
- Die Oberfläche ist deutsch lokalisiert und enthält Pfeile, Karten, Symbole, ESP-Symbole, Hilfe, Anzeige löschen und Invertiert.
- Der Testmodus deaktiviert Bluetooth vollständig und simuliert Befehle im Log. Damit kann die Oberfläche im Android-Emulator geprüft werden.
- App-Icon, Adaptive Icon, Splash Icon und Favicon wurden aus `NasreddinDerMagier.png` erzeugt.

## Development Build Verhalten

Die aktuell installierte APK ist ein Expo Development Build. Deshalb erscheint beim Start zuerst der Expo Dev Client. Dieser verbindet die native App-Hülle mit dem Metro-/Expo-Server auf dem Entwicklungsrechner.

Zusammenhang:

- Die APK enthält native Android-App, `expo-dev-client` und native Module wie `react-native-ble-plx`.
- Metro liefert während der Entwicklung den aktuellen JavaScript-/TypeScript-Code.
- Expo Go wird nicht verwendet, weil Expo Go die native BLE-Bibliothek nicht enthält.

Für eine spätere Nutzerversion muss ein Standalone-/Preview-/Release-Build ohne Dev-Client-Startfluss erstellt werden. Dann startet die App direkt ohne Expo-Verbindungsseite.

## BLE-Konfiguration

- Bluetooth-Name: `BlePrompter`
- Service-UUID: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- RX-Characteristic zum Schreiben: `6e400002-b5a3-f393-e0a9-e50e24dcca9e`
- TX-Characteristic für Antworten: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`

Die Befehle werden als UTF-8-Text ohne Zeilenumbruch geschrieben.

## Wichtige Befehle

```powershell
cd C:\dev\Nasreddins-Simple-Peek-Client2
```

```powershell
npx expo prebuild --platform android --no-install
```

```powershell
cd C:\dev\Nasreddins-Simple-Peek-Client2\android
.\gradlew.bat app:assembleDebug -x lint -x test --configure-on-demand --build-cache -PreactNativeArchitectures=arm64-v8a,x86_64 --console=plain --warning-mode=summary
```

```powershell
adb -s emulator-5554 install -r C:\dev\Nasreddins-Simple-Peek-Client2\android\app\build\outputs\apk\debug\app-debug.apk
```

```powershell
adb -s YTXOFMS8EM5LTOBA install -r C:\dev\Nasreddins-Simple-Peek-Client2\android\app\build\outputs\apk\debug\app-debug.apk
```

## Projektkontext

Die ausführliche Übergabe steht in `PROJEKTUEBERSICHT.md`. Für ein neues Codex-Projekt gibt es zusätzlich `AGENTS_NEXT.md`; diese Datei ist als Vorlage für ein anderes Projekt gedacht und soll nicht in diesem Repository versioniert werden.
