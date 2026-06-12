#include "display/M5StickDisplay.h"
#include "display/M5StickHardware.h"

#include <cstring>
#include <math.h>
#include <string>

namespace
{

constexpr int16_t DisplayWidth = 240;
constexpr int16_t DisplayHeight = 135;
constexpr int16_t CenterX = DisplayWidth / 2;
constexpr int16_t CenterY = DisplayHeight / 2;

uint16_t foregroundColor(bool inverted)
{
    return inverted ? M5StickHardware::ColorInvertForeground : M5StickHardware::ColorForeground;
}

uint16_t backgroundColor(bool inverted)
{
    return inverted ? M5StickHardware::ColorInvertBackground : M5StickHardware::ColorBackground;
}

M5GFX &getM5Display(DisplayHardware &hardware)
{
    return static_cast<M5StickHardware &>(hardware).getDisplay();
}

void prepareDisplay(DisplayHardware &hardware, bool inverted)
{
    hardware.fillScreen(backgroundColor(inverted));
    hardware.setDrawColor(foregroundColor(inverted));

    M5GFX &display = getM5Display(hardware);
    display.setTextColor(foregroundColor(inverted), backgroundColor(inverted));
    display.setTextDatum(top_left);

    static_cast<M5StickHardware &>(hardware).drawBatteryLevelLine();
}

std::string getUppercaseAsciiString(const std::string &value)
{
    std::string result = value;
    for (size_t characterIndex = 0; characterIndex < result.length(); characterIndex++)
    {
        result[characterIndex] = static_cast<char>(toupper(static_cast<unsigned char>(result[characterIndex])));
    }
    return result;
}

void drawCenteredText(DisplayHardware &hardware, const char *text, uint8_t textScale,
    int16_t centerX, int16_t centerY, bool inverted)
{
    M5GFX &display = getM5Display(hardware);
    display.setTextColor(foregroundColor(inverted), backgroundColor(inverted));
    display.setTextSize(textScale);
    display.setTextDatum(middle_center);
    display.drawString(text, centerX, centerY);
    display.setTextDatum(top_left);
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
    case 0: return isHeartPixel(normalizedX, normalizedY);
    case 1: return isDiamondPixel(normalizedX, normalizedY);
    case 2: return isClubPixel(normalizedX, normalizedY);
    case 3: return isSpadePixel(normalizedX, normalizedY);
    }

    return false;
}

void drawSuitSymbol(DisplayHardware &hardware, uint8_t suit,
    int16_t x, int16_t y, int16_t size)
{
    constexpr uint8_t gridSize = 40;
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
    constexpr int16_t waveTopY = CenterY - 44;
    constexpr int16_t waveBottomY = CenterY + 44;
    constexpr int16_t waveStepY = 4;
    constexpr int16_t waveAmplitude = 8;
    constexpr int16_t waveThickness = 2;
    constexpr float waveCycles = 2.0f;
    const int16_t waveCenters[] = {CenterX - 34, CenterX, CenterX + 34};

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

} // namespace

namespace M5StickDisplay
{

void drawArrow(DisplayHardware &hardware, CompassDirection compassDirection, bool inverted)
{
    prepareDisplay(hardware, inverted);

    int8_t horizontalStep = 0;
    int8_t verticalStep = 0;
    switch (compassDirection)
    {
    case CompassDirection::N:  verticalStep = -1; break;
    case CompassDirection::NO: horizontalStep = 1; verticalStep = -1; break;
    case CompassDirection::O:  horizontalStep = 1; break;
    case CompassDirection::SO: horizontalStep = 1; verticalStep = 1; break;
    case CompassDirection::S:  verticalStep = 1; break;
    case CompassDirection::SW: horizontalStep = -1; verticalStep = 1; break;
    case CompassDirection::W:  horizontalStep = -1; break;
    case CompassDirection::NW: horizontalStep = -1; verticalStep = -1; break;
    }

    constexpr int16_t reach = 50;
    constexpr int16_t tailReach = 36;
    constexpr int16_t headLength = 18;
    constexpr int16_t headHalfWidth = 13;
    constexpr int8_t shaftHalfWidth = 3;

    const int16_t perpendicularHorizontalStep = -verticalStep;
    const int16_t perpendicularVerticalStep = horizontalStep;

    const int16_t tipX = CenterX + horizontalStep * reach;
    const int16_t tipY = CenterY + verticalStep * reach;
    const int16_t baseX = tipX - horizontalStep * headLength;
    const int16_t baseY = tipY - verticalStep * headLength;
    const int16_t startX = CenterX - horizontalStep * tailReach;
    const int16_t startY = CenterY - verticalStep * tailReach;

    for (int8_t offset = -shaftHalfWidth; offset <= shaftHalfWidth; offset++)
    {
        hardware.drawLine(
            startX + perpendicularHorizontalStep * offset,
            startY + perpendicularVerticalStep * offset,
            baseX + perpendicularHorizontalStep * offset,
            baseY + perpendicularVerticalStep * offset);
    }

    hardware.drawTriangle(
        tipX,
        tipY,
        baseX + perpendicularHorizontalStep * headHalfWidth,
        baseY + perpendicularVerticalStep * headHalfWidth,
        baseX - perpendicularHorizontalStep * headHalfWidth,
        baseY - perpendicularVerticalStep * headHalfWidth);
}

void drawAsciiCharacters(DisplayHardware &hardware, const char *text, bool inverted)
{
    prepareDisplay(hardware, inverted);

    const std::string uppercaseText = getUppercaseAsciiString(text);
    const uint8_t characterCount = (uppercaseText.length() >= 2) ? 2u : (uppercaseText.empty() ? 0u : 1u);

    char displayCharacters[3] = {};
    if (characterCount >= 1) displayCharacters[0] = uppercaseText[0];
    if (characterCount >= 2) displayCharacters[1] = uppercaseText[1];

    drawCenteredText(hardware, displayCharacters, 6, CenterX, CenterY, inverted);
}

void drawPlayingCard(DisplayHardware &hardware, uint8_t cardIndex, bool inverted)
{
    prepareDisplay(hardware, inverted);

    if (cardIndex == 52 || cardIndex == 53)
    {
        drawCenteredText(hardware, cardIndex == 52 ? "J1" : "J2", 6, CenterX, CenterY, inverted);
        return;
    }

    const uint8_t suit = cardIndex / 13;
    const uint8_t rank = cardIndex % 13;
    const char *ranks[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "X", "J", "Q", "K"};
    const char *rankText = rank < 13 ? ranks[rank] : "?";

    hardware.setDrawColor(foregroundColor(inverted));
    drawSuitSymbol(hardware, suit, CenterX - 60, CenterY + 2, 80);
    drawCenteredText(hardware, rankText, 6, CenterX + 58, CenterY, inverted);
}

void drawDiceFace(DisplayHardware &hardware, uint8_t faceValue, bool inverted)
{
    prepareDisplay(hardware, inverted);

    constexpr int16_t dotRadius = 10;
    constexpr int16_t margin = 34;
    constexpr int16_t leftX = CenterX - margin;
    constexpr int16_t rightX = CenterX + margin;
    constexpr int16_t topY = CenterY - margin;
    constexpr int16_t middleY = CenterY;
    constexpr int16_t bottomY = CenterY + margin;

    switch (faceValue)
    {
    case 1:
        hardware.drawDisc(CenterX, CenterY, dotRadius);
        break;
    case 2:
        hardware.drawDisc(leftX, topY, dotRadius);
        hardware.drawDisc(rightX, bottomY, dotRadius);
        break;
    case 3:
        hardware.drawDisc(leftX, topY, dotRadius);
        hardware.drawDisc(CenterX, CenterY, dotRadius);
        hardware.drawDisc(rightX, bottomY, dotRadius);
        break;
    case 4:
        hardware.drawDisc(leftX, topY, dotRadius);
        hardware.drawDisc(rightX, topY, dotRadius);
        hardware.drawDisc(leftX, bottomY, dotRadius);
        hardware.drawDisc(rightX, bottomY, dotRadius);
        break;
    case 5:
        hardware.drawDisc(leftX, topY, dotRadius);
        hardware.drawDisc(rightX, topY, dotRadius);
        hardware.drawDisc(CenterX, CenterY, dotRadius);
        hardware.drawDisc(leftX, bottomY, dotRadius);
        hardware.drawDisc(rightX, bottomY, dotRadius);
        break;
    case 6:
        hardware.drawDisc(leftX, topY, dotRadius);
        hardware.drawDisc(rightX, topY, dotRadius);
        hardware.drawDisc(leftX, middleY, dotRadius);
        hardware.drawDisc(rightX, middleY, dotRadius);
        hardware.drawDisc(leftX, bottomY, dotRadius);
        hardware.drawDisc(rightX, bottomY, dotRadius);
        break;
    }
}

void drawEspSymbol(DisplayHardware &hardware, EspSymbol symbol, bool inverted)
{
    prepareDisplay(hardware, inverted);

    switch (symbol)
    {
    case EspSymbol::circle:
        for (int16_t radius = 38; radius >= 20; radius -= 3)
        {
            hardware.drawCircle(CenterX, CenterY, radius);
        }
        break;

    case EspSymbol::cross:
        hardware.drawBox(CenterX - 7, CenterY - 44, 14, 88);
        hardware.drawBox(CenterX - 44, CenterY - 7, 88, 14);
        break;

    case EspSymbol::waves:
        drawWaveSymbol(hardware);
        break;

    case EspSymbol::square:
        for (int16_t offset = 0; offset < 7; offset++)
        {
            hardware.drawFrame(
                CenterX - 36 + offset,
                CenterY - 36 + offset,
                72 - offset * 2,
                72 - offset * 2);
        }
        break;

    case EspSymbol::star:
    {
        constexpr int16_t pointCount = 5;
        constexpr int16_t starOuterRadius = 45;
        constexpr int16_t starInnerRadius = 18;

        int16_t outerX[pointCount];
        int16_t outerY[pointCount];
        int16_t innerX[pointCount];
        int16_t innerY[pointCount];

        for (int pointIndex = 0; pointIndex < pointCount; pointIndex++)
        {
            const float outerAngle = -PI / 2.0f + pointIndex * 2.0f * PI / pointCount;
            const float innerAngle = outerAngle + PI / pointCount;

            outerX[pointIndex] = CenterX + static_cast<int16_t>(cosf(outerAngle) * starOuterRadius);
            outerY[pointIndex] = CenterY + static_cast<int16_t>(sinf(outerAngle) * starOuterRadius);
            innerX[pointIndex] = CenterX + static_cast<int16_t>(cosf(innerAngle) * starInnerRadius);
            innerY[pointIndex] = CenterY + static_cast<int16_t>(sinf(innerAngle) * starInnerRadius);
        }

        for (int pointIndex = 0; pointIndex < pointCount; pointIndex++)
        {
            const int nextPointIndex = (pointIndex + 1) % pointCount;
            hardware.drawTriangle(
                outerX[pointIndex],
                outerY[pointIndex],
                innerX[pointIndex],
                innerY[pointIndex],
                outerX[nextPointIndex],
                outerY[nextPointIndex]);
        }
        break;
    }
    }
}

} // namespace M5StickDisplay
