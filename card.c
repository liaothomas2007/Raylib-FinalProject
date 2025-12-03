#include "card.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"

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
int GetCardValue(Card c) {
    int score = BASE_RANK_SCORES[c.rank];

    // 特殊規則範例：如果是黑桃分數*2
    if (c.suit == 0)
    {
       score *= 2;
    }
    
    return score;
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
    for (int i = 0; i < handSize; i++)
    {
        if (hand[i].selected)
        {
            if (count < 5) 
            {
                selectedCards[count] = hand[i];
                selectedIndices[count] = i;
                count++;
            }
        }
    }

    if (count == 0) return;

    // 2. 排序
    qsort(selectedCards, count, sizeof(Card), CompareCardsByRank);

    // 3. 計算「卡牌基礎總分」(Chips)
    float baseChips = 0;
    for (int i = 0; i < count; i++) {
        baseChips += GetCardValue(selectedCards[i]);
    }

    // 4. 判斷牌型並套用倍率 (Mult)
    bool isValidHand = false;
    float finalScore = 0.0f;
    const char* handName = "";

    // 從修改器讀取目前的倍率
    float currentMult = 1.0f;

    if (count == 1) // 單張
    {
        isValidHand = true;
        handName = "Single";
        // 基礎倍率 * 道具倍率
        currentMult = 1.0f * mods->multSingle; 
        
        // 關卡特殊規則 (Level 2 單張倍率減半)
        if (level == 2) currentMult *= 0.5f;
        if (level == 3) currentMult = 0.0f;
    }
    else if (count == 2) // 對子
    {
        if (selectedCards[0].rank == selectedCards[1].rank)
        {
            isValidHand = true;
            handName = "Pair";
            // 基礎倍率 * 道具倍率
            currentMult = 2.0f * mods->multPair;
            
            // 關卡特殊規則
            if (level == 2) currentMult *= 2.0f; // Level 2 對子加強
        }
    }

    // 5. 最終結算： (牌面總分 + 額外籌碼) * 倍率
    if (isValidHand)
    {
        finalScore = (baseChips + mods->bonusChips) * currentMult;

        printf("牌型: %s | 牌面分: %.0f | 倍率: %.1f | 總分: %.1f\n", 
               handName, baseChips, currentMult, finalScore);
        
        *score += finalScore;

        // 打出與補牌邏輯
        for (int i = 0; i < count; i++)
        {
            int handIdx = selectedIndices[i];
            hand[handIdx].played = true;
            hand[handIdx].selected = false;
            hand[handIdx].currentY = (float)HAND_START_Y;
        }
        DrawCards(deck, hand, handSize, deckTopIndex);
    }
    else
    {
        printf("無效牌型!\n");
        for (int i = 0; i < handSize; i++) {
            hand[i].selected = false;
            hand[i].targetY = (float)HAND_START_Y;
        }
    }
}