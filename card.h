#ifndef CARD_H
#define CARD_H

#include "raylib.h"
#include <stdlib.h> // 確保 malloc/free 可用
#include <stdbool.h> // 確保 bool 可用

typedef struct 
{
    int rank;
    int suit;
    bool selected;
    bool played;
    // 統一用於卡牌動畫
    float currentY; 
    float targetY;
} Card;

// 全域資源宣告（於 card.c 定義）
extern Texture2D cardTextures[4][13];
extern Texture2D card_back;

// 宣告全域常量 (定義在 card.c)
extern const int CARD_WIDTH_BASE;
extern const int CARD_HEIGHT_BASE;
extern const float CARD_SCALE;
extern const int HAND_START_X;
extern const int HAND_START_Y;
extern const int CARD_GAP;
extern const int CARD_STEP;

void InitDeck(Card* deck);
void ShuffleDeck(Card* deck, int size);
void LoadCardTextures();
void ShowDeck();
void DrawCards(Card* deck, Card* hand, int handSize, int* deckTopIndex); 
void UpdateAndDrawHand(Card* hand, int handSize);
void CheckAndScoreHand(Card* deck, Card* hand, int handSize, int* deckTopIndex, float* score);

#endif
