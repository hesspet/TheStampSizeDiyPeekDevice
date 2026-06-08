#pragma once

#include <U8g2lib.h>
#include "DisplayHardware.h"

/// SSD1306-OLED-Hardwaretreiber für das ESP32-C3-OLED-Board (72×40 Pixel).
/// Nutzt U8g2 über I2C.
class Ssd1306Hardware : public DisplayHardware
{
public:
    /// @param dataPin  I2C-SDA-Pin (z.B. GPIO5)
    /// @param clockPin I2C-SCL-Pin (z.B. GPIO6)
    Ssd1306Hardware(uint8_t dataPin, uint8_t clockPin);

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

    // --- U8g2-spezifisch ---
    U8G2 &getU8g2();

private:
    U8G2_SSD1306_72X40_ER_F_HW_I2C display;
    uint8_t dataPin;
    uint8_t clockPin;
};
