#include <StampDisplay/AsciiCharacterDisplay.h>

namespace
{
constexpr uint8_t displayWidth = 72;
constexpr uint8_t displayHeight = 40;
constexpr uint8_t displayBaselineY = 34;
}

void AsciiCharacterDisplay::drawCharacters(U8G2 &display, const char *characters, bool inverted) const
{
    if (characters == nullptr || characters[0] == '\0')
    {
        return;
    }

    drawCharacters(display, characters[0], characters[1], inverted);
}

void AsciiCharacterDisplay::drawCharacters(U8G2 &display, char firstCharacter, char secondCharacter, bool inverted) const
{
    prepareDisplay(display, inverted);

    char text[] = {
        getPrintableAsciiCharacter(firstCharacter),
        '\0',
        '\0'
    };

    if (secondCharacter != '\0')
    {
        text[1] = getPrintableAsciiCharacter(secondCharacter);
    }

    drawCenteredText(display, text);
    display.setDrawColor(1);
}

void AsciiCharacterDisplay::prepareDisplay(U8G2 &display, bool inverted) const
{
    display.setDrawColor(1);

    if (inverted)
    {
        display.drawBox(0, 0, displayWidth, displayHeight);
        display.setDrawColor(0);
    }
}

char AsciiCharacterDisplay::getPrintableAsciiCharacter(char character) const
{
    return character >= 32 && character <= 126 ? character : '?';
}

void AsciiCharacterDisplay::drawCenteredText(U8G2 &display, const char *text) const
{
    display.setFont(u8g2_font_helvB24_tf);
    const int16_t textWidth = display.getStrWidth(text);
    display.drawStr((displayWidth - textWidth) / 2, displayBaselineY, text);
}
