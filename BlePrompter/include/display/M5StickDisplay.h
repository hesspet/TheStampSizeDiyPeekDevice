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

/// Display-Symbole für den M5StickC Plus2 (TFT 240 x 135, Landscape).
namespace M5StickDisplay
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

} // namespace M5StickDisplay
