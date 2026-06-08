#pragma once

#include "display/DisplayHardware.h"

/// Kompassrichtungen für Pfeilbefehle.
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

/// ESP-Zener-Symbole.
enum class EspSymbol : uint8_t
{
    circle,
    cross,
    waves,
    square,
    star
};

/// Display-Symbole für das CYB/CYD-Board (ILI9341 TFT 320×240).
/// Entspricht funktional der StampDisplay-Bibliothek, nutzt aber TFT_eSPI-Primitive.
namespace CydDisplay
{

void drawArrow(
    DisplayHardware &hardware,
    CompassDirection compassDirection,
    bool inverted);

void drawAsciiCharacters(
    DisplayHardware &hardware,
    const char *text,
    bool inverted);

void drawPlayingCard(
    DisplayHardware &hardware,
    uint8_t cardIndex,
    bool inverted);

static constexpr uint8_t cardCount = 54;

void drawDiceFace(
    DisplayHardware &hardware,
    uint8_t faceValue,
    bool inverted);

static constexpr uint8_t faceCount = 6;

void drawEspSymbol(
    DisplayHardware &hardware,
    EspSymbol symbol,
    bool inverted);

} // namespace CydDisplay
