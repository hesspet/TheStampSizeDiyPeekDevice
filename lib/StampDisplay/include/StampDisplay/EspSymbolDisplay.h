#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

enum class EspSymbol : uint8_t
{
    circle,
    cross,
    waves,
    square,
    star
};

class EspSymbolDisplay
{
public:
    static constexpr uint8_t symbolCount = 5;

    void drawSymbol(U8G2 &display, EspSymbol symbol, bool inverted = false) const;
    const char *getSymbolDescription(EspSymbol symbol) const;

private:
    void prepareDisplay(U8G2 &display, bool inverted) const;
    void drawCircleSymbol(U8G2 &display) const;
    void drawCrossSymbol(U8G2 &display) const;
    void drawWavesSymbol(U8G2 &display) const;
    void drawSquareSymbol(U8G2 &display) const;
    void drawStarSymbol(U8G2 &display) const;
};
