#include <StampDisplay/PlayingCardDisplay.h>

#include <cstdio>
#include <math.h>

namespace
{
constexpr uint8_t displayWidth = 72;
constexpr uint8_t displayHeight = 40;
constexpr uint8_t normalCardSuitOriginX = 0;
constexpr uint8_t normalCardSuitOriginY = 2;
constexpr uint8_t normalCardSuitSize = 36;
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
    drawCardSuit(display, playingCard.suit, normalCardSuitOriginX, normalCardSuitOriginY, normalCardSuitSize);

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

void PlayingCardDisplay::drawCardSuit(
    U8G2 &display,
    PlayingCardSuit suit,
    uint8_t originX,
    uint8_t originY,
    uint8_t size) const
{
    for (uint8_t pixelY = 0; pixelY < size; pixelY++)
    {
        for (uint8_t pixelX = 0; pixelX < size; pixelX++)
        {
            if (isSuitPixel(suit, pixelX, pixelY, size))
            {
                display.drawPixel(originX + pixelX, originY + pixelY);
            }
        }
    }
}

bool PlayingCardDisplay::isSuitPixel(PlayingCardSuit suit, uint8_t pixelX, uint8_t pixelY, uint8_t size) const
{
    const float normalizedX = (((static_cast<float>(pixelX) + 0.5f) / static_cast<float>(size)) * 2.0f - 1.0f) * 1.18f;
    const float normalizedY = (1.0f - ((static_cast<float>(pixelY) + 0.5f) / static_cast<float>(size)) * 2.0f) * 1.18f;

    switch (suit)
    {
        case PlayingCardSuit::hearts:
            return isHeartPixel(normalizedX, normalizedY);
        case PlayingCardSuit::diamonds:
            return isDiamondPixel(normalizedX, normalizedY);
        case PlayingCardSuit::clubs:
            return isClubPixel(normalizedX, normalizedY);
        case PlayingCardSuit::spades:
            return isSpadePixel(normalizedX, normalizedY);
    }

    return false;
}

bool PlayingCardDisplay::isHeartPixel(float normalizedX, float normalizedY) const
{
    const float shiftedY = normalizedY + 0.10f;
    const float squareTerm = normalizedX * normalizedX + shiftedY * shiftedY - 1.0f;
    return squareTerm * squareTerm * squareTerm - normalizedX * normalizedX * shiftedY * shiftedY * shiftedY <= 0.0f
        && shiftedY > -1.05f;
}

bool PlayingCardDisplay::isDiamondPixel(float normalizedX, float normalizedY) const
{
    return fabsf(normalizedX) * 0.78f + fabsf(normalizedY) <= 0.86f;
}

bool PlayingCardDisplay::isClubPixel(float normalizedX, float normalizedY) const
{
    const bool topLobe = normalizedX * normalizedX + (normalizedY - 0.35f) * (normalizedY - 0.35f) <= 0.17f;
    const bool leftLobe = (normalizedX + 0.38f) * (normalizedX + 0.38f) + (normalizedY + 0.08f) * (normalizedY + 0.08f) <= 0.17f;
    const bool rightLobe = (normalizedX - 0.38f) * (normalizedX - 0.38f) + (normalizedY + 0.08f) * (normalizedY + 0.08f) <= 0.17f;
    const bool stem = fabsf(normalizedX) <= 0.13f && normalizedY <= -0.05f && normalizedY >= -0.78f;
    const bool foot = fabsf(normalizedX) <= 0.34f && normalizedY <= -0.70f && normalizedY >= -0.86f;
    return topLobe || leftLobe || rightLobe || stem || foot;
}

bool PlayingCardDisplay::isSpadePixel(float normalizedX, float normalizedY) const
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
