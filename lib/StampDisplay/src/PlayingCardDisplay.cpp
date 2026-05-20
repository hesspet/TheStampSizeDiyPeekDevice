#include <StampDisplay/PlayingCardDisplay.h>

#include <cstdio>

namespace
{
constexpr uint8_t displayWidth = 72;
constexpr uint8_t displayHeight = 40;
constexpr uint8_t normalCardSuitOriginX = 1;
constexpr uint8_t normalCardSuitOriginY = 3;
constexpr uint8_t normalCardValueBaselineY = 34;
constexpr uint8_t normalCardValueRightMarginX = 71;
constexpr uint8_t jokerBaselineY = 34;
}

PlayingCard PlayingCardDisplay::getCard(uint8_t cardIndex) const
{
    if (cardIndex >= cardCount - 2)
    {
        return {
            PlayingCardSuit::hearts,
            cardIndex == cardCount - 2 ? PlayingCardRank::jokerOne : PlayingCardRank::jokerTwo
        };
    }

    const uint8_t suitIndex = cardIndex / rankCountPerSuit;
    const uint8_t rankIndex = cardIndex % rankCountPerSuit;

    PlayingCardSuit suit = PlayingCardSuit::hearts;
    switch (suitIndex)
    {
        case 0:
            suit = PlayingCardSuit::hearts;
            break;
        case 1:
            suit = PlayingCardSuit::diamonds;
            break;
        case 2:
            suit = PlayingCardSuit::clubs;
            break;
        case 3:
            suit = PlayingCardSuit::spades;
            break;
    }

    return {
        suit,
        static_cast<PlayingCardRank>(rankIndex)
    };
}

const char *PlayingCardDisplay::getSuitDescription(PlayingCardSuit suit) const
{
    switch (suit)
    {
        case PlayingCardSuit::hearts:
            return "Heart";
        case PlayingCardSuit::diamonds:
            return "Diamond";
        case PlayingCardSuit::clubs:
            return "Clubs";
        case PlayingCardSuit::spades:
            return "Spade";
    }

    return "Unbekannt";
}

const char *PlayingCardDisplay::getRankDisplayText(PlayingCardRank rank) const
{
    switch (rank)
    {
        case PlayingCardRank::ace:
            return "1";
        case PlayingCardRank::two:
            return "2";
        case PlayingCardRank::three:
            return "3";
        case PlayingCardRank::four:
            return "4";
        case PlayingCardRank::five:
            return "5";
        case PlayingCardRank::six:
            return "6";
        case PlayingCardRank::seven:
            return "7";
        case PlayingCardRank::eight:
            return "8";
        case PlayingCardRank::nine:
            return "9";
        case PlayingCardRank::ten:
            return "X";
        case PlayingCardRank::jack:
            return "J";
        case PlayingCardRank::queen:
            return "Q";
        case PlayingCardRank::king:
            return "K";
        case PlayingCardRank::jokerOne:
            return "J1";
        case PlayingCardRank::jokerTwo:
            return "J2";
    }

    return "?";
}

const char *PlayingCardDisplay::getRankDescription(PlayingCardRank rank) const
{
    switch (rank)
    {
        case PlayingCardRank::ace:
            return "Ace";
        case PlayingCardRank::two:
            return "2";
        case PlayingCardRank::three:
            return "3";
        case PlayingCardRank::four:
            return "4";
        case PlayingCardRank::five:
            return "5";
        case PlayingCardRank::six:
            return "6";
        case PlayingCardRank::seven:
            return "7";
        case PlayingCardRank::eight:
            return "8";
        case PlayingCardRank::nine:
            return "9";
        case PlayingCardRank::ten:
            return "Ten";
        case PlayingCardRank::jack:
            return "Jack";
        case PlayingCardRank::queen:
            return "Queen";
        case PlayingCardRank::king:
            return "King";
        case PlayingCardRank::jokerOne:
            return "Joker 1";
        case PlayingCardRank::jokerTwo:
            return "Joker 2";
    }

    return "Unbekannt";
}

bool PlayingCardDisplay::isJoker(PlayingCardRank rank) const
{
    return rank == PlayingCardRank::jokerOne || rank == PlayingCardRank::jokerTwo;
}

void PlayingCardDisplay::getCardDescription(uint8_t cardIndex, char *descriptionBuffer, size_t descriptionBufferSize) const
{
    const PlayingCard playingCard = getCard(cardIndex % cardCount);

    if (isJoker(playingCard.rank))
    {
        snprintf(descriptionBuffer, descriptionBufferSize, "%s", getRankDescription(playingCard.rank));
        return;
    }

    snprintf(
        descriptionBuffer,
        descriptionBufferSize,
        "%s %s",
        getSuitDescription(playingCard.suit),
        getRankDescription(playingCard.rank));
}

void PlayingCardDisplay::drawCard(U8G2 &display, uint8_t cardIndex, bool inverted) const
{
    prepareDisplay(display, inverted);

    const PlayingCard playingCard = getCard(cardIndex % cardCount);

    if (isJoker(playingCard.rank))
    {
        drawJokerCard(display, playingCard.rank);
        display.setDrawColor(1);
        return;
    }

    drawNormalCard(display, playingCard);
    display.setDrawColor(1);
}

void PlayingCardDisplay::prepareDisplay(U8G2 &display, bool inverted) const
{
    display.setDrawColor(1);

    if (inverted)
    {
        display.drawBox(0, 0, displayWidth, displayHeight);
        display.setDrawColor(0);
    }
}

void PlayingCardDisplay::drawNormalCard(U8G2 &display, const PlayingCard &playingCard) const
{
    drawCardSuit(display, playingCard.suit, normalCardSuitOriginX, normalCardSuitOriginY);

    display.setFont(u8g2_font_helvB24_tf);
    const char *rankText = getRankDisplayText(playingCard.rank);
    const uint8_t rankTextWidth = display.getStrWidth(rankText);
    display.drawStr(normalCardValueRightMarginX - rankTextWidth, normalCardValueBaselineY, rankText);
}

void PlayingCardDisplay::drawJokerCard(U8G2 &display, PlayingCardRank rank) const
{
    display.setFont(u8g2_font_helvB24_tf);
    const char *jokerText = getRankDisplayText(rank);
    const uint8_t jokerTextWidth = display.getStrWidth(jokerText);
    display.drawStr((displayWidth - jokerTextWidth) / 2, jokerBaselineY, jokerText);
}

void PlayingCardDisplay::drawCardSuit(U8G2 &display, PlayingCardSuit suit, uint8_t originX, uint8_t originY) const
{
    switch (suit)
    {
        case PlayingCardSuit::hearts:
            drawHeartSuit(display, originX, originY);
            break;
        case PlayingCardSuit::diamonds:
            drawDiamondSuit(display, originX, originY);
            break;
        case PlayingCardSuit::clubs:
            drawClubSuit(display, originX, originY);
            break;
        case PlayingCardSuit::spades:
            drawSpadeSuit(display, originX, originY);
            break;
    }
}

void PlayingCardDisplay::drawHeartSuit(U8G2 &display, uint8_t originX, uint8_t originY) const
{
    display.drawDisc(originX + 10, originY + 10, 10, U8G2_DRAW_ALL);
    display.drawDisc(originX + 24, originY + 10, 10, U8G2_DRAW_ALL);
    display.drawTriangle(originX + 1, originY + 13, originX + 33, originY + 13, originX + 17, originY + 33);
}

void PlayingCardDisplay::drawDiamondSuit(U8G2 &display, uint8_t originX, uint8_t originY) const
{
    display.drawTriangle(originX + 17, originY, originX + 34, originY + 17, originX, originY + 17);
    display.drawTriangle(originX + 17, originY + 34, originX + 34, originY + 17, originX, originY + 17);
}

void PlayingCardDisplay::drawClubSuit(U8G2 &display, uint8_t originX, uint8_t originY) const
{
    display.drawDisc(originX + 17, originY + 9, 9, U8G2_DRAW_ALL);
    display.drawDisc(originX + 9, originY + 21, 9, U8G2_DRAW_ALL);
    display.drawDisc(originX + 25, originY + 21, 9, U8G2_DRAW_ALL);
    display.drawBox(originX + 14, originY + 21, 7, 14);
    display.drawBox(originX + 10, originY + 32, 15, 4);
}

void PlayingCardDisplay::drawSpadeSuit(U8G2 &display, uint8_t originX, uint8_t originY) const
{
    display.drawDisc(originX + 10, originY + 21, 10, U8G2_DRAW_ALL);
    display.drawDisc(originX + 24, originY + 21, 10, U8G2_DRAW_ALL);
    display.drawTriangle(originX + 2, originY + 21, originX + 32, originY + 21, originX + 17, originY);
    display.drawBox(originX + 14, originY + 22, 7, 13);
    display.drawBox(originX + 10, originY + 32, 15, 4);
}
