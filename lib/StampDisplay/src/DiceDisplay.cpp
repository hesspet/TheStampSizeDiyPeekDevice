#include <StampDisplay/DiceDisplay.h>

namespace
{
constexpr uint8_t displayWidth = 72;
constexpr uint8_t displayHeight = 40;

constexpr int16_t centerX = 36;
constexpr int16_t centerY = 20;

constexpr int16_t dotOffsetX = 14;
constexpr int16_t dotOffsetY = 10;

constexpr int16_t leftX = centerX - dotOffsetX;
constexpr int16_t rightX = centerX + dotOffsetX;
constexpr int16_t topY = centerY - dotOffsetY;
constexpr int16_t middleY = centerY;
constexpr int16_t bottomY = centerY + dotOffsetY;

constexpr uint8_t dotRadius = 5;
constexpr uint8_t dotOptions = U8G2_DRAW_ALL;
}

void DiceDisplay::drawFace(U8G2 &display, uint8_t faceValue, bool inverted) const
{
    if (faceValue < 1 || faceValue > faceCount)
    {
        return;
    }

    prepareDisplay(display, inverted);
    drawDiceFace(display, faceValue);
    display.setDrawColor(1);
}

const char *DiceDisplay::getFaceDescription(uint8_t faceValue) const
{
    switch (faceValue)
    {
        case 1:
            return "Würfel 1";
        case 2:
            return "Würfel 2";
        case 3:
            return "Würfel 3";
        case 4:
            return "Würfel 4";
        case 5:
            return "Würfel 5";
        case 6:
            return "Würfel 6";
    }

    return "Würfel unbekannt";
}

void DiceDisplay::prepareDisplay(U8G2 &display, bool inverted) const
{
    display.setDrawColor(1);

    if (inverted)
    {
        display.drawBox(0, 0, displayWidth, displayHeight);
        display.setDrawColor(0);
    }
}

void DiceDisplay::drawDot(U8G2 &display, int16_t x, int16_t y) const
{
    display.drawDisc(x, y, dotRadius, dotOptions);
}

void DiceDisplay::drawDiceFace(U8G2 &display, uint8_t faceValue) const
{
    switch (faceValue)
    {
        case 1:
            drawDot(display, centerX, centerY);
            break;

        case 2:
            drawDot(display, leftX, bottomY);
            drawDot(display, rightX, topY);
            break;

        case 3:
            drawDot(display, leftX, bottomY);
            drawDot(display, centerX, centerY);
            drawDot(display, rightX, topY);
            break;

        case 4:
            drawDot(display, leftX, topY);
            drawDot(display, rightX, topY);
            drawDot(display, leftX, bottomY);
            drawDot(display, rightX, bottomY);
            break;

        case 5:
            drawDot(display, leftX, topY);
            drawDot(display, rightX, topY);
            drawDot(display, centerX, centerY);
            drawDot(display, leftX, bottomY);
            drawDot(display, rightX, bottomY);
            break;

        case 6:
            drawDot(display, leftX, topY);
            drawDot(display, rightX, topY);
            drawDot(display, leftX, middleY);
            drawDot(display, rightX, middleY);
            drawDot(display, leftX, bottomY);
            drawDot(display, rightX, bottomY);
            break;
    }
}