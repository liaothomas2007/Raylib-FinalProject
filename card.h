#ifndef CARD_H
#define CARD_H

#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h> // 確保 bool 可用

// --- 1. 定義魔法卡的效果類型 ---
typedef enum {
    EFFECT_NONE,
    EFFECT_MULT_SINGLE, // 增加單張倍率
    EFFECT_MULT_PAIR,   // 增加對子倍率
    EFFECT_BONUS_CHIPS, // 增加基礎籌碼 (所有牌型加分)
} MagicEffectType;

// --- 2. 定義魔法卡結構 ---
typedef struct {
    char name[32];          // 卡片名稱
    char description[64];   // 功能描述
    MagicEffectType type;   // 效果類型
    float value;            // 效果數值 (例如 +0.5 或 +10)
    Color color;            // 卡片顏色 (視覺區分)
} MagicCard;

// --- 3. 定義遊戲數值修改器 (玩家身上的 Buff) ---
typedef struct {
    float multSingle;   // 單張倍率 (預設 1.0)
    float multPair;     // 對子倍率 (預設 1.0)
    int bonusChips;     // 額外籌碼 (預設 0)
    // 之後可擴充更多 (如順子倍率等)
} GameModifiers;

// --- 4. 原本的卡牌結構 ---
typedef struct 
{
    int rank;
    int suit;
    bool selected;
    bool played;
    float currentY; 
    float targetY;
} Card;

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

// --- 遊戲數值修改器結構 ---
typedef struct {
    float multSingle;   // 單張倍率
    float multPair;     // 對子倍率
    float multStraight;      // 順子倍率
    float multFlush;         // 同花倍率
    float multFullHouse;     // 葫蘆倍率 
    float multFourOfAKind;   // 鐵支倍率
    float multStraightFlush; // 同花順倍率
    int bonusChips;     // 額外籌碼 (例如：所有手牌 +10 分)
} GameModifiers;

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
void DrawCards(Card* deck, Card* hand, int handSize, int* deckTopIndex); 
void UpdateAndDrawHand(Card* hand, int handSize);
void CheckAndScoreHand(Card* deck, Card* hand, int handSize, int* deckTopIndex, float* score, int level, GameModifiers* mods);
#endif