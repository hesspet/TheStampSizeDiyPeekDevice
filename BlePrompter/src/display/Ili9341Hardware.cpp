#include "display/Ili9341Hardware.h"

Ili9341Hardware::Ili9341Hardware()
{
}

void Ili9341Hardware::begin()
{
    tft.begin();
    tft.setRotation(1); // Landscape, USB-Anschluss rechts
    tft.fillScreen(ColorBackground);
    tft.setTextColor(ColorForeground, ColorBackground);
    pinMode(27, OUTPUT);
    digitalWrite(27, HIGH);
    backlightOn = true;
}

void Ili9341Hardware::clearBuffer()
{
    // TFT_eSPI hat keinen Framebuffer. fillScreen als Ersatz.
    tft.fillScreen(ColorBackground);
}

void Ili9341Hardware::sendBuffer()
{
    // TFT zeichnet direkt — kein Puffer-Flush nötig.
}

uint8_t *Ili9341Hardware::getBufferPtr()
{
    return nullptr; // TFT hat keinen zugreifbaren Framebuffer.
}

size_t Ili9341Hardware::getBufferSize()
{
    return 0;
}

void Ili9341Hardware::setFont(const void *font)
{
    // Font-ID wird als uintptr_t übergeben.
    activeFontId = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(font));
    tft.setTextFont(activeFontId);
}

void Ili9341Hardware::setFontMode(uint8_t mode)
{
    // TFT_eSPI hat kein Font-Mode-Konzept.
    // Textfarben werden über setTextColor gesteuert.
}

void Ili9341Hardware::enableUTF8Print()
{
    // TFT_eSPI behandelt UTF-8 nativ.
}

void Ili9341Hardware::setCursor(int16_t x, int16_t y)
{
    tft.setCursor(x, y);
}

void Ili9341Hardware::print(const char *text)
{
    tft.print(text);
}

void Ili9341Hardware::print(int32_t value)
{
    tft.print(value);
}

int16_t Ili9341Hardware::getStrWidth(const char *text)
{
    return tft.textWidth(text);
}

int16_t Ili9341Hardware::getDisplayWidth()
{
    return tft.width();
}

int16_t Ili9341Hardware::getDisplayHeight()
{
    return tft.height();
}

void Ili9341Hardware::setDrawColor(uint16_t color)
{
    drawColor = color;
}

void Ili9341Hardware::drawBox(int16_t x, int16_t y, int16_t w, int16_t h)
{
    tft.fillRect(x, y, w, h, drawColor);
}

void Ili9341Hardware::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    tft.drawLine(x1, y1, x2, y2, drawColor);
}

void Ili9341Hardware::drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3)
{
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, drawColor);
}

void Ili9341Hardware::drawCircle(int16_t x, int16_t y, int16_t radius)
{
    tft.drawCircle(x, y, radius, drawColor);
}

void Ili9341Hardware::drawDisc(int16_t x, int16_t y, int16_t radius)
{
    tft.fillCircle(x, y, radius, drawColor);
}

void Ili9341Hardware::drawFrame(int16_t x, int16_t y, int16_t w, int16_t h)
{
    tft.drawRect(x, y, w, h, drawColor);
}

void Ili9341Hardware::drawPixel(int16_t x, int16_t y)
{
    tft.drawPixel(x, y, drawColor);
}

void Ili9341Hardware::fillScreen(uint16_t color)
{
    tft.fillScreen(color);
    tft.setTextColor(ColorForeground, color);
}

void Ili9341Hardware::setDisplayRotation(uint8_t rotation)
{
    tft.setRotation(rotation);
}

void Ili9341Hardware::enterHardwareSleep()
{
    digitalWrite(27, LOW);
    backlightOn = false;
}

void Ili9341Hardware::wakeFromHardwareSleep()
{
    digitalWrite(27, HIGH);
    backlightOn = true;
}

void Ili9341Hardware::deactivateBeforeDeepSleep()
{
    tft.fillScreen(TFT_BLACK);
    digitalWrite(27, LOW);
    backlightOn = false;
}

void *Ili9341Hardware::getRawDisplay()
{
    return &tft;
}

TFT_eSPI &Ili9341Hardware::getTft()
{
    return tft;
}
