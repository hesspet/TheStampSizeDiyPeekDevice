#include "display/CydDisplay.h"
#include "display/Ili9341Hardware.h"

#include <cstring>
#include <string>

namespace
{

constexpr int16_t DisplayWidth = 320;
constexpr int16_t DisplayHeight = 240;
constexpr int16_t CenterX = 160;
constexpr int16_t CenterY = 120;

uint16_t foregroundColor(bool inverted)
{
    return inverted ? Ili9341Hardware::ColorInvertForeground : Ili9341Hardware::ColorForeground;
}

uint16_t backgroundColor(bool inverted)
{
    return inverted ? Ili9341Hardware::ColorInvertBackground : Ili9341Hardware::ColorBackground;
}

void prepareDisplay(DisplayHardware &hardware, bool inverted)
{
    hardware.fillScreen(backgroundColor(inverted));
    hardware.setDrawColor(foregroundColor(inverted));
}

std::string getUppercaseAsciiString(const std::string &value)
{
    std::string result = value;
    for (size_t i = 0; i < result.length(); i++)
    {
        result[i] = static_cast<char>(toupper(static_cast<unsigned char>(result[i])));
    }
    return result;
}

} // namespace

// ========================================================================
// Pfeile
// ========================================================================

namespace CydDisplay
{

void drawArrow(DisplayHardware &hardware, CompassDirection compassDirection, bool inverted)
{
    prepareDisplay(hardware, inverted);

    int8_t hStep = 0;
    int8_t vStep = 0;
    switch (compassDirection)
    {
    case CompassDirection::N:  vStep = -1; break;
    case CompassDirection::NO: hStep = 1; vStep = -1; break;
    case CompassDirection::O:  hStep = 1; break;
    case CompassDirection::SO: hStep = 1; vStep = 1; break;
    case CompassDirection::S:  vStep = 1; break;
    case CompassDirection::SW: hStep = -1; vStep = 1; break;
    case CompassDirection::W:  hStep = -1; break;
    case CompassDirection::NW: hStep = -1; vStep = -1; break;
    }

    constexpr int16_t reach = 100;
    constexpr int16_t tailReach = 65;
    constexpr int16_t headLength = 35;
    constexpr int16_t headHalfWidth = 22;
    constexpr int8_t shaftHalfWidth = 6;

    const int16_t pHS = -vStep; // perpendicular horizontal
    const int16_t pVS = hStep;  // perpendicular vertical

    const int16_t tipX = CenterX + hStep * reach;
    const int16_t tipY = CenterY + vStep * reach;
    const int16_t baseX = tipX - hStep * headLength;
    const int16_t baseY = tipY - vStep * headLength;
    const int16_t startX = CenterX - hStep * tailReach;
    const int16_t startY = CenterY - vStep * tailReach;

    // Schaft
    for (int8_t off = -shaftHalfWidth; off <= shaftHalfWidth; off++)
    {
        hardware.drawLine(
            startX + pHS * off, startY + pVS * off,
            baseX + pHS * off, baseY + pVS * off);
    }

    // Spitze
    hardware.drawTriangle(
        tipX, tipY,
        baseX + pHS * headHalfWidth, baseY + pVS * headHalfWidth,
        baseX - pHS * headHalfWidth, baseY - pVS * headHalfWidth);
}

// ========================================================================
// ASCII-Zeichen
// ========================================================================

void drawAsciiCharacters(DisplayHardware &hardware, const char *text, bool inverted)
{
    prepareDisplay(hardware, inverted);

    const std::string upper = getUppercaseAsciiString(text);
    const uint8_t charCount = (upper.length() >= 2) ? 2u : (upper.empty() ? 0u : 1u);

    char displayChars[3] = {};
    if (charCount >= 1) displayChars[0] = upper[0];
    if (charCount >= 2) displayChars[1] = upper[1];

    // Großer Font für Symbole
    hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(7)));
    const int16_t textWidth = hardware.getStrWidth(displayChars);
    hardware.setCursor((DisplayWidth - textWidth) / 2, CenterY - 20);
    hardware.print(displayChars);
}

// ========================================================================
// Spielkarten
// ========================================================================

const char *getCardDescription(uint8_t cardIndex, char *buffer, size_t bufferSize)
{
    if (cardIndex >= 54)
    {
        snprintf(buffer, bufferSize, "???");
        return buffer;
    }

    if (cardIndex == 52)
    {
        snprintf(buffer, bufferSize, "J1");
        return buffer;
    }
    if (cardIndex == 53)
    {
        snprintf(buffer, bufferSize, "J2");
        return buffer;
    }

    const char *suits[] = {"H", "D", "C", "S"};
    const char *ranks[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "X", "J", "Q", "K"};

    const uint8_t suit = cardIndex / 13;
    const uint8_t rank = cardIndex % 13;

    if (suit < 4 && rank < 13)
    {
        snprintf(buffer, bufferSize, "%s%s", suits[suit], ranks[rank]);
    }
    else
    {
        snprintf(buffer, bufferSize, "???");
    }

    return buffer;
}

void drawSuitSymbol(DisplayHardware &hardware, uint8_t suit,
    int16_t x, int16_t y, int16_t size)
{
    const int16_t half = size / 2;
    const int16_t quarter = size / 4;

    switch (suit)
    {
    case 0: // Hearts
        hardware.drawDisc(x - quarter, y - quarter, quarter);
        hardware.drawDisc(x + quarter, y - quarter, quarter);
        hardware.drawTriangle(x - half, y, x + half, y, x, y - size);
        break;

    case 1: // Diamonds
        hardware.drawTriangle(x, y - half, x + half, y, x, y + half);
        hardware.drawTriangle(x - half, y, x + half, y, x, y - half);
        break;

    case 2: // Clubs
        hardware.drawDisc(x - quarter, y - quarter + 3, quarter);
        hardware.drawDisc(x + quarter, y - quarter + 3, quarter);
        hardware.drawDisc(x, y - half + 6, quarter);
        hardware.drawBox(x - 3, y + 4, 7, half);
        break;

    case 3: // Spades
        hardware.drawDisc(x - quarter, y - quarter + 3, quarter);
        hardware.drawDisc(x + quarter, y - quarter + 3, quarter);
        hardware.drawTriangle(x - half + 2, y, x + half - 2, y, x, y - half);
        hardware.drawBox(x - 3, y + 2, 7, half + 2);
        break;
    }
}

void drawPlayingCard(DisplayHardware &hardware, uint8_t cardIndex, bool inverted)
{
    prepareDisplay(hardware, inverted);

    char cardText[8];
    getCardDescription(cardIndex, cardText, sizeof(cardText));

    if (cardIndex == 52 || cardIndex == 53)
    {
        // Joker: nur Text
        hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(7)));
        const int16_t tw = hardware.getStrWidth(cardText);
        hardware.setCursor((DisplayWidth - tw) / 2, CenterY - 20);
        hardware.print(cardText);
        return;
    }

    const uint8_t suit = cardIndex / 13;
    const char *ranks[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "X", "J", "Q", "K"};
    const char *rankText = (cardIndex % 13 < 13) ? ranks[cardIndex % 13] : "?";

    // Linker Bereich: Kartensymbol
    hardware.setDrawColor(foregroundColor(inverted));
    drawSuitSymbol(hardware, suit, CenterX - 70, CenterY, 60);

    // Rechter Bereich: Wert als Text
    hardware.setFont(reinterpret_cast<const void *>(static_cast<uintptr_t>(7)));
    const int16_t tw = hardware.getStrWidth(rankText);
    hardware.setCursor(CenterX + 30 - tw / 2, CenterY - 20);
    hardware.print(rankText);
}

// ========================================================================
// Würfel
// ========================================================================

void drawDot(DisplayHardware &hardware, int16_t x, int16_t y, int16_t radius)
{
    hardware.drawDisc(x, y, radius);
}

void drawDiceFace(DisplayHardware &hardware, uint8_t faceValue, bool inverted)
{
    prepareDisplay(hardware, inverted);

    constexpr int16_t dotRadius = 16;
    constexpr int16_t margin = 55;
    constexpr int16_t leftX = CenterX - margin;
    constexpr int16_t rightX = CenterX + margin;
    constexpr int16_t topY = CenterY - margin;
    constexpr int16_t midY = CenterY;
    constexpr int16_t botY = CenterY + margin;

    // Würfelrahmen
    hardware.drawFrame(leftX - 30, topY - 30, 120, 120);

    switch (faceValue)
    {
    case 1:
        drawDot(hardware, CenterX, CenterY, dotRadius);
        break;
    case 2:
        drawDot(hardware, leftX, topY, dotRadius);
        drawDot(hardware, rightX, botY, dotRadius);
        break;
    case 3:
        drawDot(hardware, leftX, topY, dotRadius);
        drawDot(hardware, CenterX, CenterY, dotRadius);
        drawDot(hardware, rightX, botY, dotRadius);
        break;
    case 4:
        drawDot(hardware, leftX, topY, dotRadius);
        drawDot(hardware, rightX, topY, dotRadius);
        drawDot(hardware, leftX, botY, dotRadius);
        drawDot(hardware, rightX, botY, dotRadius);
        break;
    case 5:
        drawDot(hardware, leftX, topY, dotRadius);
        drawDot(hardware, rightX, topY, dotRadius);
        drawDot(hardware, CenterX, CenterY, dotRadius);
        drawDot(hardware, leftX, botY, dotRadius);
        drawDot(hardware, rightX, botY, dotRadius);
        break;
    case 6:
        drawDot(hardware, leftX, topY, dotRadius);
        drawDot(hardware, rightX, topY, dotRadius);
        drawDot(hardware, leftX, midY, dotRadius);
        drawDot(hardware, rightX, midY, dotRadius);
        drawDot(hardware, leftX, botY, dotRadius);
        drawDot(hardware, rightX, botY, dotRadius);
        break;
    }
}

// ========================================================================
// ESP-Symbole
// ========================================================================

void drawEspSymbol(DisplayHardware &hardware, EspSymbol symbol, bool inverted)
{
    prepareDisplay(hardware, inverted);

    switch (symbol)
    {
    case EspSymbol::circle:
        for (int16_t r = 60; r >= 30; r -= 3)
        {
            hardware.drawCircle(CenterX, CenterY, r);
        }
        break;

    case EspSymbol::cross:
        hardware.drawBox(CenterX - 10, CenterY - 70, 20, 140);
        hardware.drawBox(CenterX - 70, CenterY - 10, 140, 20);
        break;

    case EspSymbol::waves:
        for (int16_t i = 0; i < 8; i++)
        {
            const int16_t x = CenterX - 70 + i * 20;
            hardware.drawLine(x, CenterY + 50, x + 10, CenterY - 50);
            hardware.drawLine(x, CenterY - 50, x + 10, CenterY + 50);
        }
        break;

    case EspSymbol::square:
        for (int16_t off = 0; off < 10; off++)
        {
            hardware.drawFrame(
                CenterX - 55 + off, CenterY - 55 + off,
                110 - off * 2, 110 - off * 2);
        }
        break;

    case EspSymbol::star:
    {
        const int16_t starOuter = 70;
        const int16_t starInner = 28;
        constexpr int16_t numPoints = 5;

        int16_t outerX[numPoints];
        int16_t outerY[numPoints];
        int16_t innerX[numPoints];
        int16_t innerY[numPoints];

        for (int i = 0; i < numPoints; i++)
        {
            const float angle = -PI / 2.0f + i * 2.0f * PI / numPoints;
            const float innerAngle = angle + PI / numPoints;

            outerX[i] = CenterX + static_cast<int16_t>(cos(angle) * starOuter);
            outerY[i] = CenterY + static_cast<int16_t>(sin(angle) * starOuter);
            innerX[i] = CenterX + static_cast<int16_t>(cos(innerAngle) * starInner);
            innerY[i] = CenterY + static_cast<int16_t>(sin(innerAngle) * starInner);
        }

        for (int i = 0; i < numPoints; i++)
        {
            const int next = (i + 1) % numPoints;
            hardware.drawTriangle(
                outerX[i], outerY[i],
                innerX[i], innerY[i],
                outerX[next], outerY[next]);
        }
        break;
    }
    }
}

} // namespace CydDisplay
