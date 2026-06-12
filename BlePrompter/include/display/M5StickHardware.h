#pragma once

#include <M5Unified.h>
#include "DisplayHardware.h"

/// TFT-Hardwaretreiber für den M5StickC Plus2 (240 x 135 Pixel, Landscape).
class M5StickHardware : public DisplayHardware
{
public:
    M5StickHardware();

    void begin() override;
    void clearBuffer() override;
    void sendBuffer() override;
    uint8_t *getBufferPtr() override;
    size_t getBufferSize() override;

    void setFont(const void *font) override;
    void setFontMode(uint8_t mode) override;
    void enableUTF8Print() override;
    void setCursor(int16_t x, int16_t y) override;
    void print(const char *text) override;
    void print(int32_t value) override;
    int16_t getStrWidth(const char *text) override;

    int16_t getDisplayWidth() override;
    int16_t getDisplayHeight() override;

    void setDrawColor(uint16_t color) override;
    void drawBox(int16_t x, int16_t y, int16_t w, int16_t h) override;
    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) override;
    void drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) override;
    void drawCircle(int16_t x, int16_t y, int16_t radius) override;
    void drawDisc(int16_t x, int16_t y, int16_t radius) override;
    void drawFrame(int16_t x, int16_t y, int16_t w, int16_t h) override;
    void drawPixel(int16_t x, int16_t y) override;
    void fillScreen(uint16_t color) override;

    void setDisplayRotation(uint8_t rotation) override;
    void enterHardwareSleep() override;
    void wakeFromHardwareSleep() override;
    void deactivateBeforeDeepSleep() override;

    void *getRawDisplay() override;

    M5GFX &getDisplay();
    int32_t getBatteryLevelPercent();
    int16_t getBatteryVoltageMillivolts();
    void drawBatteryLevelLine();

    static constexpr uint16_t ColorBackground = 0x0000;       // Schwarz
    static constexpr uint16_t ColorForeground = 0x07E0;       // Grün
    static constexpr uint16_t ColorInvertBackground = 0x07E0; // Grün
    static constexpr uint16_t ColorInvertForeground = 0x0000; // Schwarz
    static constexpr uint16_t ColorBatteryWarning = 0xFFE0;   // Gelb
    static constexpr uint16_t ColorBatteryCritical = 0xF800;  // Rot
    static constexpr uint16_t ColorBatteryTrack = 0x39E7;     // Dunkelgrau

private:
    uint16_t drawColor = ColorForeground;
};
