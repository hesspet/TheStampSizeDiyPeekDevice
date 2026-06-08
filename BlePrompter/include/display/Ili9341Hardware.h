#pragma once

#include <TFT_eSPI.h>
#include "DisplayHardware.h"

/// ILI9341-TFT-Hardwaretreiber für das CYB/CYD-Board (320×240 Pixel).
/// Nutzt TFT_eSPI über SPI. Konfiguration via TFT_eSPI_Setup_CYD.h.
class Ili9341Hardware : public DisplayHardware
{
public:
    Ili9341Hardware();

    // --- DisplayHardware ---
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

    // --- TFT_eSPI-spezifisch ---
    TFT_eSPI &getTft();

    /// Farben für den CYB-Farbmodus (schwarzer Hintergrund, gelbe Schrift).
    static constexpr uint16_t ColorBackground = TFT_BLACK;
    static constexpr uint16_t ColorForeground = TFT_YELLOW;
    static constexpr uint16_t ColorInvertBackground = TFT_YELLOW;
    static constexpr uint16_t ColorInvertForeground = TFT_BLACK;

private:
    TFT_eSPI tft;
    uint16_t drawColor = ColorForeground;
    uint8_t activeFontId = 2;
    bool backlightOn = true;
};
