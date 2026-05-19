#include <StampDisplay/ArrowDisplay.h>

namespace
{
constexpr int16_t displayCenterX = 36;
constexpr int16_t displayCenterY = 20;
constexpr uint8_t displayWidth = 72;
constexpr uint8_t displayHeight = 40;
constexpr int16_t horizontalArrowReach = 28;
constexpr int16_t verticalArrowReach = 16;
constexpr int16_t horizontalTailReach = 18;
constexpr int16_t verticalTailReach = 11;
constexpr int16_t arrowHeadLength = 10;
constexpr int16_t arrowHeadHalfWidth = 6;
constexpr int8_t arrowShaftHalfWidth = 2;
}

void ArrowDisplay::drawArrow(U8G2 &display, CompassDirection compassDirection, bool inverted) const
{
    prepareDisplay(display, inverted);

    const DirectionVector directionVector = getDirectionVector(compassDirection);
    const int16_t perpendicularHorizontalStep = -directionVector.verticalStep;
    const int16_t perpendicularVerticalStep = directionVector.horizontalStep;

    const int16_t arrowEndX = displayCenterX + directionVector.horizontalStep * horizontalArrowReach;
    const int16_t arrowEndY = displayCenterY + directionVector.verticalStep * verticalArrowReach;
    const int16_t arrowStartX = displayCenterX - directionVector.horizontalStep * horizontalTailReach;
    const int16_t arrowStartY = displayCenterY - directionVector.verticalStep * verticalTailReach;
    const int16_t arrowHeadBaseX = arrowEndX - directionVector.horizontalStep * arrowHeadLength;
    const int16_t arrowHeadBaseY = arrowEndY - directionVector.verticalStep * arrowHeadLength;

    for (int8_t shaftOffset = -arrowShaftHalfWidth; shaftOffset <= arrowShaftHalfWidth; shaftOffset++)
    {
        display.drawLine(
            arrowStartX + perpendicularHorizontalStep * shaftOffset,
            arrowStartY + perpendicularVerticalStep * shaftOffset,
            arrowHeadBaseX + perpendicularHorizontalStep * shaftOffset,
            arrowHeadBaseY + perpendicularVerticalStep * shaftOffset);
    }

    display.drawTriangle(
        arrowEndX,
        arrowEndY,
        arrowHeadBaseX + perpendicularHorizontalStep * arrowHeadHalfWidth,
        arrowHeadBaseY + perpendicularVerticalStep * arrowHeadHalfWidth,
        arrowHeadBaseX - perpendicularHorizontalStep * arrowHeadHalfWidth,
        arrowHeadBaseY - perpendicularVerticalStep * arrowHeadHalfWidth);

    display.setDrawColor(1);
}

const char *ArrowDisplay::getDirectionDescription(CompassDirection compassDirection) const
{
    switch (compassDirection)
    {
        case CompassDirection::N:
            return "Pfeil nach Norden";
        case CompassDirection::NO:
            return "Pfeil nach Nordosten";
        case CompassDirection::O:
            return "Pfeil nach Osten";
        case CompassDirection::SO:
            return "Pfeil nach Südosten";
        case CompassDirection::S:
            return "Pfeil nach Süden";
        case CompassDirection::SW:
            return "Pfeil nach Südwesten";
        case CompassDirection::W:
            return "Pfeil nach Westen";
        case CompassDirection::NW:
            return "Pfeil nach Nordwesten";
    }

    return "Pfeil unbekannt";
}

void ArrowDisplay::prepareDisplay(U8G2 &display, bool inverted) const
{
    display.setDrawColor(1);

    if (inverted)
    {
        display.drawBox(0, 0, displayWidth, displayHeight);
        display.setDrawColor(0);
    }
}

ArrowDisplay::DirectionVector ArrowDisplay::getDirectionVector(CompassDirection compassDirection) const
{
    switch (compassDirection)
    {
        case CompassDirection::N:
            return {0, -1};
        case CompassDirection::NO:
            return {1, -1};
        case CompassDirection::O:
            return {1, 0};
        case CompassDirection::SO:
            return {1, 1};
        case CompassDirection::S:
            return {0, 1};
        case CompassDirection::SW:
            return {-1, 1};
        case CompassDirection::W:
            return {-1, 0};
        case CompassDirection::NW:
            return {-1, -1};
    }

    return {0, -1};
}
