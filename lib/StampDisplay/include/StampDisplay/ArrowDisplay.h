#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

enum class CompassDirection : uint8_t
{
    N,
    NO,
    O,
    SO,
    S,
    SW,
    W,
    NW
};

class ArrowDisplay
{
public:
    static constexpr uint8_t directionCount = 8;

    void drawArrow(U8G2 &display, CompassDirection compassDirection, bool inverted = false) const;
    const char *getDirectionDescription(CompassDirection compassDirection) const;

private:
    struct DirectionVector
    {
        int8_t horizontalStep;
        int8_t verticalStep;
    };

    void prepareDisplay(U8G2 &display, bool inverted) const;
    DirectionVector getDirectionVector(CompassDirection compassDirection) const;
};
