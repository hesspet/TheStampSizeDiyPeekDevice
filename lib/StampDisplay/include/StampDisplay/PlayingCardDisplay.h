#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

enum class PlayingCardSuit : uint8_t
{
    hearts,
    diamonds,
    clubs,
    spades
};

enum class PlayingCardRank : uint8_t
{
    ace,
    two,
    three,
    four,
    five,
    six,
    seven,
    eight,
    nine,
    ten,
    jack,
    queen,
    king,
    jokerOne,
    jokerTwo
};

struct PlayingCard
{
    PlayingCardSuit suit;
    PlayingCardRank rank;
};

class PlayingCardDisplay
{
public:
    static constexpr uint8_t cardCount = 54;

    void drawCard(U8G2 &display, uint8_t cardIndex, bool inverted = false) const;
    void getCardDescription(uint8_t cardIndex, char *descriptionBuffer, size_t descriptionBufferSize) const;

private:
    static constexpr uint8_t suitCount = 4;
    static constexpr uint8_t rankCountPerSuit = 13;

    PlayingCard getCard(uint8_t cardIndex) const;
    const char *getSuitDescription(PlayingCardSuit suit) const;
    const char *getRankDisplayText(PlayingCardRank rank) const;
    const char *getRankDescription(PlayingCardRank rank) const;
    bool isJoker(PlayingCardRank rank) const;

    void prepareDisplay(U8G2 &display, bool inverted) const;
    void drawNormalCard(U8G2 &display, const PlayingCard &playingCard) const;
    void drawJokerCard(U8G2 &display, PlayingCardRank rank) const;
    void drawCardSuit(U8G2 &display, PlayingCardSuit suit, uint8_t originX, uint8_t originY) const;
    void drawHeartSuit(U8G2 &display, uint8_t originX, uint8_t originY) const;
    void drawDiamondSuit(U8G2 &display, uint8_t originX, uint8_t originY) const;
    void drawClubSuit(U8G2 &display, uint8_t originX, uint8_t originY) const;
    void drawSpadeSuit(U8G2 &display, uint8_t originX, uint8_t originY) const;
};
