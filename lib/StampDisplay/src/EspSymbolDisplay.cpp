#include <StampDisplay/EspSymbolDisplay.h>

namespace
{
constexpr uint8_t displayWidth = 72;
constexpr uint8_t displayHeight = 40;
constexpr int16_t displayCenterX = 36;
constexpr int16_t displayCenterY = 20;
}

void EspSymbolDisplay::drawSymbol(U8G2 &display, EspSymbol symbol, bool inverted) const
{
    prepareDisplay(display, inverted);

    switch (symbol)
    {
        case EspSymbol::circle:
            drawCircleSymbol(display);
            break;
        case EspSymbol::cross:
            drawCrossSymbol(display);
            break;
        case EspSymbol::waves:
            drawWavesSymbol(display);
            break;
        case EspSymbol::square:
            drawSquareSymbol(display);
            break;
        case EspSymbol::star:
            drawStarSymbol(display);
            break;
    }

    display.setDrawColor(1);
}

const char *EspSymbolDisplay::getSymbolDescription(EspSymbol symbol) const
{
    switch (symbol)
    {
        case EspSymbol::circle:
            return "ESP-Kreis";
        case EspSymbol::cross:
            return "ESP-Kreuz";
        case EspSymbol::waves:
            return "ESP-Wellen";
        case EspSymbol::square:
            return "ESP-Quadrat";
        case EspSymbol::star:
            return "ESP-Stern";
    }

    return "ESP-Symbol unbekannt";
}

void EspSymbolDisplay::prepareDisplay(U8G2 &display, bool inverted) const
{
    display.setDrawColor(1);

    if (inverted)
    {
        display.drawBox(0, 0, displayWidth, displayHeight);
        display.setDrawColor(0);
    }
}

void EspSymbolDisplay::drawCircleSymbol(U8G2 &display) const
{
    display.drawCircle(displayCenterX, displayCenterY, 15, U8G2_DRAW_ALL);
    display.drawCircle(displayCenterX, displayCenterY, 14, U8G2_DRAW_ALL);
    display.drawCircle(displayCenterX, displayCenterY, 13, U8G2_DRAW_ALL);
    display.drawCircle(displayCenterX, displayCenterY, 12, U8G2_DRAW_ALL);
    display.drawCircle(displayCenterX, displayCenterY, 11, U8G2_DRAW_ALL);
}

void EspSymbolDisplay::drawCrossSymbol(U8G2 &display) const
{
    display.drawBox(displayCenterX - 4, 5, 9, 30);
    display.drawBox(displayCenterX - 18, displayCenterY - 4, 37, 9);
}

void EspSymbolDisplay::drawWavesSymbol(U8G2 &display) const
{
    const int16_t waveOrigins[] = {22, 36, 50};

    for (uint8_t waveIndex = 0; waveIndex < 3; waveIndex++)
    {
        const int16_t originX = waveOrigins[waveIndex];

        for (int8_t thicknessOffset = -2; thicknessOffset <= 2; thicknessOffset++)
        {
            display.drawLine(originX + thicknessOffset, 5, originX + 4 + thicknessOffset, 9);
            display.drawLine(originX + 4 + thicknessOffset, 9, originX - 4 + thicknessOffset, 15);
            display.drawLine(originX - 4 + thicknessOffset, 15, originX + 4 + thicknessOffset, 21);
            display.drawLine(originX + 4 + thicknessOffset, 21, originX - 4 + thicknessOffset, 27);
            display.drawLine(originX - 4 + thicknessOffset, 27, originX + thicknessOffset, 35);
        }
    }
}

void EspSymbolDisplay::drawSquareSymbol(U8G2 &display) const
{
    display.drawFrame(21, 5, 30, 30);
    display.drawFrame(22, 6, 28, 28);
    display.drawFrame(23, 7, 26, 26);
    display.drawFrame(24, 8, 24, 24);
    display.drawFrame(25, 9, 22, 22);
}

void EspSymbolDisplay::drawStarSymbol(U8G2 &display) const
{
    const int16_t topX = 36;
    const int16_t topY = 4;
    const int16_t rightX = 51;
    const int16_t rightY = 15;
    const int16_t lowerRightX = 45;
    const int16_t lowerRightY = 33;
    const int16_t lowerLeftX = 27;
    const int16_t lowerLeftY = 33;
    const int16_t leftX = 21;
    const int16_t leftY = 15;

    display.drawLine(topX, topY, lowerRightX, lowerRightY);
    display.drawLine(lowerRightX, lowerRightY, leftX, leftY);
    display.drawLine(leftX, leftY, rightX, rightY);
    display.drawLine(rightX, rightY, lowerLeftX, lowerLeftY);
    display.drawLine(lowerLeftX, lowerLeftY, topX, topY);

    display.drawLine(topX - 1, topY + 1, lowerRightX - 1, lowerRightY);
    display.drawLine(lowerRightX - 1, lowerRightY, leftX, leftY + 1);
    display.drawLine(leftX, leftY + 1, rightX, rightY + 1);
    display.drawLine(rightX, rightY + 1, lowerLeftX + 1, lowerLeftY);
    display.drawLine(lowerLeftX + 1, lowerLeftY, topX + 1, topY + 1);

    display.drawLine(topX - 2, topY + 2, lowerRightX - 2, lowerRightY);
    display.drawLine(lowerRightX - 2, lowerRightY, leftX, leftY + 2);
    display.drawLine(leftX, leftY + 2, rightX, rightY + 2);
    display.drawLine(rightX, rightY + 2, lowerLeftX + 2, lowerLeftY);
    display.drawLine(lowerLeftX + 2, lowerLeftY, topX + 2, topY + 2);
}
