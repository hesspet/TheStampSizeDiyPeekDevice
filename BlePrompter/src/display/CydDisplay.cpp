#include "display/CydDisplay.h"
#include "display/Ili9341Hardware.h"

#include <cstring>
#include <math.h>
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

    TFT_eSPI &tft = static_cast<Ili9341Hardware &>(hardware).getTft();
    tft.setTextColor(foregroundColor(inverted), backgroundColor(inverted), true);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(nullptr);
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

bool isHeartPixel(float normalizedX, float normalizedY)
{
    const float shiftedY = normalizedY + 0.10f;
    const float squareTerm = normalizedX * normalizedX + shiftedY * shiftedY - 1.0f;
    return squareTerm * squareTerm * squareTerm - normalizedX * normalizedX * shiftedY * shiftedY * shiftedY <= 0.0f
        && shiftedY > -1.05f;
}

bool isDiamondPixel(float normalizedX, float normalizedY)
{
    return fabsf(normalizedX) * 0.78f + fabsf(normalizedY) <= 0.86f;
}

bool isClubPixel(float normalizedX, float normalizedY)
{
    const bool topLobe = normalizedX * normalizedX + (normalizedY - 0.35f) * (normalizedY - 0.35f) <= 0.17f;
    const bool leftLobe = (normalizedX + 0.38f) * (normalizedX + 0.38f) + (normalizedY + 0.08f) * (normalizedY + 0.08f) <= 0.17f;
    const bool rightLobe = (normalizedX - 0.38f) * (normalizedX - 0.38f) + (normalizedY + 0.08f) * (normalizedY + 0.08f) <= 0.17f;
    const bool stem = fabsf(normalizedX) <= 0.13f && normalizedY <= -0.05f && normalizedY >= -0.78f;
    const bool foot = fabsf(normalizedX) <= 0.34f && normalizedY <= -0.70f && normalizedY >= -0.86f;
    return topLobe || leftLobe || rightLobe || stem || foot;
}

bool isSpadePixel(float normalizedX, float normalizedY)
{
    const bool pointedTop = normalizedY >= 0.08f
        && normalizedY <= 0.92f
        && fabsf(normalizedX) <= (0.96f - normalizedY) * 0.64f;
    const bool leftShoulder = (normalizedX + 0.34f) * (normalizedX + 0.34f)
        + (normalizedY + 0.08f) * (normalizedY + 0.08f) <= 0.24f;
    const bool rightShoulder = (normalizedX - 0.34f) * (normalizedX - 0.34f)
        + (normalizedY + 0.08f) * (normalizedY + 0.08f) <= 0.24f;
    const bool middleFill = fabsf(normalizedX) <= 0.36f && normalizedY >= -0.18f && normalizedY <= 0.28f;
    const bool stem = fabsf(normalizedX) <= 0.13f && normalizedY <= -0.12f && normalizedY >= -0.78f;
    const bool foot = fabsf(normalizedX) <= 0.34f && normalizedY <= -0.70f && normalizedY >= -0.86f;
    return pointedTop || leftShoulder || rightShoulder || middleFill || stem || foot;
}

bool isSuitPixel(uint8_t suit, uint8_t pixelX, uint8_t pixelY, uint8_t gridSize)
{
    const float normalizedX = (((static_cast<float>(pixelX) + 0.5f) / static_cast<float>(gridSize)) * 2.0f - 1.0f) * 1.18f;
    const float normalizedY = (1.0f - ((static_cast<float>(pixelY) + 0.5f) / static_cast<float>(gridSize)) * 2.0f) * 1.18f;

    switch (suit)
    {
    case 0:
        return isHeartPixel(normalizedX, normalizedY);
    case 1:
        return isDiamondPixel(normalizedX, normalizedY);
    case 2:
        return isClubPixel(normalizedX, normalizedY);
    case 3:
        return isSpadePixel(normalizedX, normalizedY);
    }

    return false;
}

void drawCenteredText(DisplayHardware &hardware, const char *text, const GFXfont *font,
    uint8_t textScale, int16_t x, int16_t y, bool inverted)
{
    TFT_eSPI &tft = static_cast<Ili9341Hardware &>(hardware).getTft();
    tft.setTextColor(foregroundColor(inverted), backgroundColor(inverted), true);
    tft.setTextSize(textScale);
    tft.setFreeFont(font);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(text, x, y);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(nullptr);
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

    drawCenteredText(hardware, displayChars, &FreeSansBold24pt7b, 2, CenterX, CenterY, inverted);
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
    constexpr uint8_t gridSize = 52;
    const int16_t pixelSize = size / gridSize;
    const int16_t effectiveSize = pixelSize * gridSize;
    const int16_t originX = x - effectiveSize / 2;
    const int16_t originY = y - effectiveSize / 2;

    for (uint8_t pixelY = 0; pixelY < gridSize; pixelY++)
    {
        for (uint8_t pixelX = 0; pixelX < gridSize; pixelX++)
        {
            if (isSuitPixel(suit, pixelX, pixelY, gridSize))
            {
                hardware.drawBox(
                    originX + pixelX * pixelSize,
                    originY + pixelY * pixelSize,
                    pixelSize,
                    pixelSize);
            }
        }
    }
}

void drawPlayingCard(DisplayHardware &hardware, uint8_t cardIndex, bool inverted)
{
    prepareDisplay(hardware, inverted);

    char cardText[8];
    getCardDescription(cardIndex, cardText, sizeof(cardText));

    if (cardIndex == 52 || cardIndex == 53)
    {
        drawCenteredText(hardware, cardText, &FreeSansBold24pt7b, 2, CenterX, CenterY, inverted);
        return;
    }

    const uint8_t suit = cardIndex / 13;
    const char *ranks[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "X", "J", "Q", "K"};
    const char *rankText = (cardIndex % 13 < 13) ? ranks[cardIndex % 13] : "?";

    // Linker Bereich: Kartensymbol
    hardware.setDrawColor(foregroundColor(inverted));
    drawSuitSymbol(hardware, suit, CenterX - 86, CenterY + 6, 104);

    // Rechter Bereich: Wert als Text
    drawCenteredText(hardware, rankText, &FreeSansBold24pt7b, 2, CenterX + 82, CenterY, inverted);
}

// ========================================================================
// Würfel
// ========================================================================

void drawDot(DisplayHardware &hardware, int16_t x, int16_t y, int16_t radius)
{
    hardware.drawDisc(x, y, radius);
}

void drawThickLine(DisplayHardware &hardware, int16_t startX, int16_t startY, int16_t endX, int16_t endY, int16_t radius)
{
    for (int16_t offsetX = -radius; offsetX <= radius; offsetX++)
    {
        for (int16_t offsetY = -radius; offsetY <= radius; offsetY++)
        {
            if (offsetX * offsetX + offsetY * offsetY <= radius * radius)
            {
                hardware.drawLine(
                    startX + offsetX,
                    startY + offsetY,
                    endX + offsetX,
                    endY + offsetY);
            }
        }
    }
}

void drawWaveSymbol(DisplayHardware &hardware)
{
    constexpr int16_t waveTopY = CenterY - 70;
    constexpr int16_t waveBottomY = CenterY + 70;
    constexpr int16_t waveStepY = 4;
    constexpr int16_t waveAmplitude = 13;
    constexpr int16_t waveThickness = 4;
    constexpr float waveCycles = 2.0f;
    const int16_t waveCenters[] = {CenterX - 52, CenterX, CenterX + 52};

    for (uint8_t waveIndex = 0; waveIndex < 3; waveIndex++)
    {
        const int16_t centerX = waveCenters[waveIndex];
        int16_t previousX = centerX;
        int16_t previousY = waveTopY;

        for (int16_t currentY = waveTopY + waveStepY; currentY <= waveBottomY; currentY += waveStepY)
        {
            const float progress = static_cast<float>(currentY - waveTopY) / static_cast<float>(waveBottomY - waveTopY);
            const int16_t currentX = centerX + static_cast<int16_t>(sinf(progress * waveCycles * 2.0f * PI) * waveAmplitude);
            drawThickLine(hardware, previousX, previousY, currentX, currentY, waveThickness);
            previousX = currentX;
            previousY = currentY;
        }
    }
}

void drawDiceFace(DisplayHardware &hardware, uint8_t faceValue, bool inverted)
{
    prepareDisplay(hardware, inverted);

    constexpr int16_t dotRadius = 20;
    constexpr int16_t margin = 62;
    constexpr int16_t leftX = CenterX - margin;
    constexpr int16_t rightX = CenterX + margin;
    constexpr int16_t topY = CenterY - margin;
    constexpr int16_t midY = CenterY;
    constexpr int16_t botY = CenterY + margin;

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
        drawWaveSymbol(hardware);
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
