#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

class AsciiCharacterDisplay
{
public:
    void drawCharacters(U8G2 &display, const char *characters, bool inverted = false) const;
    void drawCharacters(U8G2 &display, char firstCharacter, char secondCharacter = '\0', bool inverted = false) const;

private:
    void prepareDisplay(U8G2 &display, bool inverted) const;
    char getPrintableAsciiCharacter(char character) const;
    void drawCenteredText(U8G2 &display, const char *text) const;
};
