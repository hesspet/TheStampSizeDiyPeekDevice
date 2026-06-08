#pragma once

#include <Arduino.h>

/// Abstrakte Basisklasse für Display-Hardware.
/// Jedes unterstützte Board implementiert diese Schnittstelle.
class DisplayHardware
{
public:
    virtual ~DisplayHardware() = default;

    // --- Lebenszyklus ---
    virtual void begin() = 0;

    // --- Puffer ---
    virtual void clearBuffer() = 0;
    virtual void sendBuffer() = 0;
    virtual uint8_t *getBufferPtr() = 0;
    virtual size_t getBufferSize() = 0;

    // --- Text ---
    virtual void setFont(const void *font) = 0;
    virtual void setFontMode(uint8_t mode) = 0;
    virtual void enableUTF8Print() = 0;
    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void print(const char *text) = 0;
    virtual void print(int32_t value) = 0;
    virtual int16_t getStrWidth(const char *text) = 0;

    // --- Abmessungen ---
    virtual int16_t getDisplayWidth() = 0;
    virtual int16_t getDisplayHeight() = 0;

    // --- Zeichenprimitive ---
    virtual void setDrawColor(uint16_t color) = 0;
    virtual void drawBox(int16_t x, int16_t y, int16_t w, int16_t h) = 0;
    virtual void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) = 0;
    virtual void drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) = 0;
    virtual void drawCircle(int16_t x, int16_t y, int16_t radius) = 0;
    virtual void drawDisc(int16_t x, int16_t y, int16_t radius) = 0;
    virtual void drawFrame(int16_t x, int16_t y, int16_t w, int16_t h) = 0;
    virtual void drawPixel(int16_t x, int16_t y) = 0;
    virtual void fillScreen(uint16_t color) = 0;

    // --- Rotation ---
    virtual void setDisplayRotation(uint8_t rotation) = 0;

    // --- Schlaf ---
    virtual void enterHardwareSleep() = 0;
    virtual void wakeFromHardwareSleep() = 0;
    virtual void deactivateBeforeDeepSleep() = 0;

    // --- Board-spezifischer Zugriff ---
    /// Liefert den rohen U8g2-Display-Pointer (nur OLED).
    /// Auf anderen Boards nullptr.
    virtual void *getRawDisplay() = 0;
};
