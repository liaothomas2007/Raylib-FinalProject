#include "card.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <raylib.h>

// 全域資源定義（配合 card.h 的 extern）
Texture2D cardTextures[4][13];
Texture2D card_back;

// 統一常數定義 (這些常量供 card.h 外部引用)
const int CARD_WIDTH_BASE = 100;
const int CARD_HEIGHT_BASE = 145;
const float CARD_SCALE = 0.3f; 
const int HAND_START_X = 75;  
const int HAND_START_Y = 680;  
const int CARD_GAP = 130;      
const int CARD_STEP = (int)(CARD_WIDTH_BASE * CARD_SCALE + CARD_GAP);

// --- 卡牌基礎分數表 (對應 Rank 0~12) ---
const int BASE_RANK_SCORES[13] = {8,1,1,1,2,2,2,3,3,3,5,5,5};

// 輔助函式：取得單張卡牌的分數
int GetCardValue(int rank) {
    // A=11, 2=2, 3=3 ... 10,J,Q,K=10
    if (rank == 0) return 11; // Ace
    if (rank >= 9) return 10; // 10, J, Q, K
    return rank + 1;          // 2~9
}

void LoadCardTextures() 
{
    for (int s = 0; s < 4; s++) 
    {
        for (int r = 0; r < 13; r++) 
        {
            char path[64];
            sprintf(path, "assets/poker cards/%d-%d.png", s, r + 1);
            cardTextures[s][r] = LoadTexture(path);
        }
    }
    card_back = LoadTexture("assets/poker cards/card_back.png");
}

void InitDeck(Card* deck) //建立主牌組
{   
    int index = 0;
    for (int s = 0; s < 4; s++) 
    {
        for (int r = 0; r < 13; r++) 
        {
            deck[index].suit = s;
            deck[index].rank = r;
            deck[index].selected = false;
            deck[index].played = false;
            deck[index].targetY = (float)HAND_START_Y; 
            deck[index].currentY = (float)HAND_START_Y;
            index++;
        }
    }
}

void ShuffleDeck(Card* deck, int size) //洗牌 (使用指標)
{
    srand((unsigned int)time(0));

    for (int i = size - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    } 
}

void DrawCards(Card* deck, Card* hand, int handSize, int* deckTopIndex) //抽牌 (配合動態記憶體與指標)
{
    for (int i = 0; i < handSize; i++) 
    {
        // 如果手牌中有一張牌被打出 (played=true)，且牌庫還有牌
        if (hand[i].played && *deckTopIndex < 52) 
        {
            hand[i] = deck[*deckTopIndex];
            hand[i].played = false; // 重置狀態
            hand[i].selected = false;
            hand[i].currentY = (float)HAND_START_Y; // 重置動畫位置
            hand[i].targetY = (float)HAND_START_Y;
            (*deckTopIndex)++; // 牌庫頂端指標移動
        }
    }
}

void UpdateAndDrawHand(Card* hand, int handSize)
{
    Vector2 mouse = GetMousePosition();
    
    for (int i = 0; i < handSize; i++)
    {
        if (hand[i].played) continue; // 打出的牌不繪製也不互動

        // 計算這張牌的座標
        float x = (float)(HAND_START_X + i * CARD_STEP);
        float y = hand[i].currentY; // 使用當前動畫Y座標

        Texture2D tex = cardTextures[hand[i].suit][hand[i].rank];
        float cardW = tex.width * CARD_SCALE;
        float cardH = tex.height * CARD_SCALE;

        // 定義碰撞框
        Rectangle rect = (Rectangle){ x, y, cardW, cardH };

        // 1. 處理輸入 (Input)
        if (CheckCollisionPointRec(mouse, rect))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                hand[i].selected = !hand[i].selected;
                // 設定動畫目標：選中時上浮 30 像素
                hand[i].targetY = hand[i].selected ? (float)(HAND_START_Y - 30) : (float)HAND_START_Y;
            }
        }

        // 2. 處理動畫 (Update) - 簡單的線性插值 (Lerp)
        hand[i].currentY += (hand[i].targetY - hand[i].currentY) * 0.2f;

        // 3. 繪製 (Draw)
        Color tint = hand[i].selected ? YELLOW : WHITE; 
        DrawTextureEx(cardTextures[hand[i].suit][hand[i].rank], (Vector2){x, hand[i].currentY}, 0, CARD_SCALE, tint);
        
        DrawRectangleLinesEx(rect, 2, BLACK);
    }

}

// 輔助：判斷是否為同花 (Flush)
bool IsFlush(Card cards[], int count) {
    if (count != 5) return false;
    int suit = cards[0].suit;
    for (int i = 1; i < count; i++) {
        if (cards[i].suit != suit) return false;
    }
    return true;
}

bool IsStraight(Card cards[], int count) {
    if (count != 5) return false;
    for (int i = 0; i < count - 1; i++) {
        // 因為已經排序過，後一張的 rank 必須等於前一張 + 1
        if (cards[i+1].rank != cards[i].rank + 1) {
            // 特殊處理：如果是 10, J, Q, K, A (Rank 9, 10, 11, 12, 0)
            // 排序後會變成 0, 9, 10, 11, 12 (A, 10, J, Q, K)
            if (i == 0 && cards[0].rank == 0 && cards[1].rank == 9 && 
                cards[2].rank == 10 && cards[3].rank == 11 && cards[4].rank == 12) {
                return true;
            }
            return false;
        }
    }
    return true;
}

// 1. 比較函式 (為了讓 qsort 知道如何排列卡牌)
int CompareCardsByRank(const void* a, const void* b)
{
    const Card* cardA = (const Card*)a;
    const Card* cardB = (const Card*)b;
    return cardA->rank - cardB->rank;
}

// 2. 核心函式：檢查牌型並計分
void CheckAndScoreHand(Card* deck, Card* hand, int handSize, int* deckTopIndex, float* score, int level, GameModifiers* mods)
{
    Card selectedCards[5];
    int selectedIndices[5];
    int count = 0;

    // 1. 收集選中的牌
    for (int i = 0; i < handSize; i++) {
        if (hand[i].selected) {
            if (count < 5) {
                selectedCards[count] = hand[i];
                selectedIndices[count] = i;
                count++;
            }
        }
    }

    if (count == 0) return;

    // 2. 排序 (判斷牌型的基礎)
    qsort(selectedCards, count, sizeof(Card), CompareCardsByRank);

    // 3. 計算基礎牌面分 (Chips)
    float baseChips = 0;
    for (int i = 0; i < count; i++) {
        baseChips += GetCardValue(selectedCards[i].rank);
    }

    // 4. 判斷牌型並設定倍率
    bool isValidHand = false;
    const char* handName = "";
    float currentMult = 0.0f;

    // --- 單張與對子 ---
    if (count == 1) {
        isValidHand = true;
        handName = "Single";
        currentMult = 1.0f * mods->multSingle;
        if (level == 2) currentMult *= 0.5f;
        if (level == 3) currentMult = 0.0f;
    }
    else if (count == 2) {
        if (selectedCards[0].rank == selectedCards[1].rank) {
            isValidHand = true;
            handName = "Pair";
            currentMult = 2.0f * mods->multPair;
            if (level == 2) currentMult *= 2.0f;
        }
    }
    // --- 5 張牌的複雜牌型 ---
    else if (count == 5) {
        bool flush = IsFlush(selectedCards, count);
        bool straight = IsStraight(selectedCards, count);

        // A. 同花順 (Straight Flush) - 12分 / 8倍 (範例)
        if (flush && straight) {
            isValidHand = true;
            handName = "Straight Flush";
            currentMult = 8.0f * mods->multStraightFlush;
        }
        // B. 鐵支 (Four of a Kind) - 10分 / 7倍 (範例)
        // 排序後只可能是 [A A A A B] 或 [B A A A A]
        else if ((selectedCards[0].rank == selectedCards[3].rank) || 
                 (selectedCards[1].rank == selectedCards[4].rank)) {
            isValidHand = true;
            handName = "Four of a Kind";
            currentMult = 7.0f * mods->multFourOfAKind;
        }
        // C. 葫蘆 (Full House) - 8分 / 4倍 (範例)
        // 排序後只可能是 [A A A B B] 或 [A A B B B]
        else if ((selectedCards[0].rank == selectedCards[2].rank && selectedCards[3].rank == selectedCards[4].rank) ||
                 (selectedCards[0].rank == selectedCards[1].rank && selectedCards[2].rank == selectedCards[4].rank)) {
            isValidHand = true;
            handName = "Full House";
            currentMult = 4.0f * mods->multFullHouse;
        }
        // D. 同花 (Flush) - 6分 / 3倍 (範例)
        else if (flush) {
            isValidHand = true;
            handName = "Flush";
            currentMult = 3.0f * mods->multFlush;
        }
        // E. 順子 (Straight) - 5分 / 2倍 (範例)
        else if (straight) {
            isValidHand = true;
            handName = "Straight";
            currentMult = 2.0f * mods->multStraight;
        }
    }

    // 5. 結算與補牌
    if (isValidHand) {
        float finalScore = (baseChips + mods->bonusChips) * currentMult;
        printf("牌型: %s | 基礎分: %.0f | 倍率: %.1f | 總分: %.1f\n", handName, baseChips, currentMult, finalScore);
        
        *score += finalScore;

        for (int i = 0; i < count; i++) {
            int idx = selectedIndices[i];
            hand[idx].played = true;
            hand[idx].selected = false;
            hand[idx].currentY = (float)HAND_START_Y;
        }
        DrawCards(deck, hand, handSize, deckTopIndex);
    } else {
        printf("無效牌型! (選了 %d 張)\n", count);
        for (int i = 0; i < handSize; i++) {
            hand[i].selected = false;
            hand[i].targetY = (float)HAND_START_Y;
        }
    }
}