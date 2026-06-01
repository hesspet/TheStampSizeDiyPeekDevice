#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

class DiceDisplay
{
public:
    static constexpr uint8_t faceCount = 6;

    void drawFace(U8G2 &display, uint8_t faceValue, bool inverted = false) const;
    const char *getFaceDescription(uint8_t faceValue) const;

private:
    void prepareDisplay(U8G2 &display, bool inverted) const;
    void drawDot(U8G2 &display, int16_t x, int16_t y) const;
    void drawDiceFace(U8G2 &display, uint8_t faceValue) const;
};