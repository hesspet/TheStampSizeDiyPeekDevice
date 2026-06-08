#include "display/Ssd1306Hardware.h"
#include <Wire.h>

Ssd1306Hardware::Ssd1306Hardware(uint8_t dataPin, uint8_t clockPin)
    : display(U8G2_R0, U8X8_PIN_NONE)
    , dataPin(dataPin)
    , clockPin(clockPin)
{
}

void Ssd1306Hardware::begin()
{
    Wire.begin(dataPin, clockPin);
    display.begin();
    display.enableUTF8Print();
}

void Ssd1306Hardware::clearBuffer()
{
    display.clearBuffer();
}

void Ssd1306Hardware::sendBuffer()
{
    display.sendBuffer();
}

uint8_t *Ssd1306Hardware::getBufferPtr()
{
    return display.getBufferPtr();
}

size_t Ssd1306Hardware::getBufferSize()
{
    return static_cast<size_t>(display.getBufferTileWidth()) * 8 * display.getBufferTileHeight();
}

void Ssd1306Hardware::setFont(const void *font)
{
    display.setFont(static_cast<const uint8_t *>(font));
}

void Ssd1306Hardware::setFontMode(uint8_t mode)
{
    display.setFontMode(mode);
}

void Ssd1306Hardware::enableUTF8Print()
{
    display.enableUTF8Print();
}

void Ssd1306Hardware::setCursor(int16_t x, int16_t y)
{
    display.setCursor(x, y);
}

void Ssd1306Hardware::print(const char *text)
{
    display.print(text);
}

void Ssd1306Hardware::print(int32_t value)
{
    display.print(value);
}

int16_t Ssd1306Hardware::getStrWidth(const char *text)
{
    return display.getStrWidth(text);
}

int16_t Ssd1306Hardware::getDisplayWidth()
{
    return display.getDisplayWidth();
}

int16_t Ssd1306Hardware::getDisplayHeight()
{
    return display.getDisplayHeight();
}

void Ssd1306Hardware::setDrawColor(uint16_t color)
{
    display.setDrawColor(color);
}

void Ssd1306Hardware::drawBox(int16_t x, int16_t y, int16_t w, int16_t h)
{
    display.drawBox(x, y, w, h);
}

void Ssd1306Hardware::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    display.drawLine(x1, y1, x2, y2);
}

void Ssd1306Hardware::drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3)
{
    display.drawTriangle(x1, y1, x2, y2, x3, y3);
}

void Ssd1306Hardware::drawCircle(int16_t x, int16_t y, int16_t radius)
{
    display.drawCircle(x, y, radius, U8G2_DRAW_ALL);
}

void Ssd1306Hardware::drawDisc(int16_t x, int16_t y, int16_t radius)
{
    display.drawDisc(x, y, radius, U8G2_DRAW_ALL);
}

void Ssd1306Hardware::drawFrame(int16_t x, int16_t y, int16_t w, int16_t h)
{
    display.drawFrame(x, y, w, h);
}

void Ssd1306Hardware::drawPixel(int16_t x, int16_t y)
{
    display.drawPixel(x, y);
}

void Ssd1306Hardware::fillScreen(uint16_t color)
{
    if (color == 0)
    {
        clearBuffer();
    }
    else
    {
        setDrawColor(1);
        drawBox(0, 0, getDisplayWidth(), getDisplayHeight());
    }
}

void Ssd1306Hardware::setDisplayRotation(uint8_t rotation)
{
    // Map einfache Werte auf U8G2-Rotationskonstanten.
    // 0 = normal (U8G2_R0), 2 = 180° (U8G2_R2).
    if (rotation == 2)
    {
        display.setDisplayRotation(U8G2_R2);
    }
    else
    {
        display.setDisplayRotation(U8G2_R0);
    }
}

void Ssd1306Hardware::enterHardwareSleep()
{
    // SSD1306: Charge-Pump deaktivieren, Panel abschalten, I2C freigeben.
    display.sendF("cac", 0x8D, 0x10, 0xAE);
    Wire.end();
}

void Ssd1306Hardware::wakeFromHardwareSleep()
{
    Wire.begin(dataPin, clockPin);
    display.begin();
    display.enableUTF8Print();
    display.setPowerSave(0);
}

void Ssd1306Hardware::deactivateBeforeDeepSleep()
{
    display.clearBuffer();
    display.sendBuffer();
    // SSD1306: Charge-Pump deaktivieren, Panel abschalten, I2C freigeben.
    display.sendF("cac", 0x8D, 0x10, 0xAE);
    Wire.end();
}

void *Ssd1306Hardware::getRawDisplay()
{
    return &display;
}

U8G2 &Ssd1306Hardware::getU8g2()
{
    return display;
}
