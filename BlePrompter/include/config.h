#pragma once

#include <Arduino.h>

// ========================================================================
// Board-Erkennung
// ========================================================================
// BOARD_CYD und BOARD_M5STICKCPLUS2 werden via platformio.ini build_flags gesetzt.
// Ohne Board-Makro wird das ESP32-C3-OLED-Board angenommen.

enum class DebugLevel : uint8_t
{
    none = 0,
    info = 1,
    debug = 2,
    trace = 3
};

// ========================================================================
// Gemeinsame Konfiguration
// ========================================================================

constexpr const char *programName = "BlePrompter";
constexpr const char *programVersion = "1.10.0";
constexpr const char *bluetoothDeviceName = "BlePrompter";
constexpr DebugLevel configuredDebugLevel = DebugLevel::info;

constexpr unsigned long serialBaudRate = 115200;
constexpr bool mirrorDebugToUsbSerial = true;
constexpr unsigned long usbSerialStartupWaitMillis = 3000;

constexpr unsigned long startupScreenDurationMillis = 2500;
constexpr bool startCycleSleepOnPowerOn = true;
constexpr bool startCycleSleepAfterBluetoothDisconnect = true;
constexpr uint32_t defaultCycleSleepSeconds = 30;
constexpr uint32_t defaultCycleListenSeconds = 10;
constexpr uint32_t minimumCycleSleepSeconds = 5;
constexpr uint32_t maximumCycleSleepSeconds = 60;
constexpr uint32_t minimumCycleListenSeconds = 10;
constexpr uint32_t maximumCycleListenSeconds = 120;
constexpr uint32_t cycleLongListenEveryCycleCount = 5;
constexpr uint32_t cycleLongListenSeconds = 60;
constexpr uint8_t maximumCommandLength = 120;

// ========================================================================
// Board-spezifische Konfiguration
// ========================================================================

#ifdef BOARD_CYD

// --- CYB/CYD-Board: ESP32-WROOM-32 mit ILI9341 TFT 320×240 ---
// Display-Pins werden über TFT_eSPI_Setup_CYD.h konfiguriert.
// Keine I2C-Display-Pins nötig.

constexpr uint8_t buttonPin = 0;            // BOOT-Button auf GPIO0
constexpr uint8_t buttonPressedLevel = LOW; // gedrückt = LOW
constexpr unsigned long buttonDebounceDurationMillis = 50;
constexpr unsigned long deepSleepButtonHoldDurationMillis = 5000;
constexpr unsigned long deepSleepCountdownIntervalMillis = 1000;

constexpr uint8_t clearDisplayMarkerSizePixels = 4;

// CYD hat kein zyklisches Tiefschlaf-Modell (kein OLED-Stromsparmodus nötig).
// Tiefschlaf-Funktionen bleiben via BLE-Befehle verfügbar,
// aber der Standard-Power-On-Zyklus ist deaktiviert.
constexpr bool startCycleSleepOnPowerOn_Cyd = false;

#elif defined(BOARD_M5STICKCPLUS2)

// --- M5StickC Plus2: ESP32-PICO mit TFT 240×135 im Landscape-Format ---
// Display und Stromversorgung werden über M5Unified initialisiert.

constexpr uint8_t buttonPin = 37;           // Front-Button A
constexpr uint8_t buttonPressedLevel = LOW; // gedrückt = LOW
constexpr unsigned long buttonDebounceDurationMillis = 50;
constexpr unsigned long deepSleepButtonHoldDurationMillis = 5000;
constexpr unsigned long deepSleepCountdownIntervalMillis = 1000;

constexpr uint8_t clearDisplayMarkerSizePixels = 4;

// Der kleine Akku des M5StickC Plus2 erfordert zyklischen Schlaf wie beim OLED-Board.
constexpr bool startCycleSleepOnPowerOn_M5StickCPlus2 = true;

#else

// --- ESP32-C3-OLED-Board: SSD1306 72×40 via I2C ---

constexpr uint8_t displayDataPin = 5;
constexpr uint8_t displayClockPin = 6;

constexpr uint8_t buttonPin = 9;
constexpr uint8_t buttonPressedLevel = LOW;
constexpr unsigned long buttonDebounceDurationMillis = 50;
constexpr unsigned long deepSleepButtonHoldDurationMillis = 5000;
constexpr unsigned long deepSleepCountdownIntervalMillis = 1000;

constexpr uint8_t clearDisplayMarkerSizePixels = 2;

#endif
