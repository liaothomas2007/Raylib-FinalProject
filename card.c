// card.c
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
            deck[index].targetY = HAND_START_Y; 
            deck[index].currentY = HAND_START_Y;
            index++;
        }
    }
}

void ShuffleDeck(Card* deck, int size) //洗牌 (使用指標)
{
    srand(time(0));

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
            hand[i].currentY = HAND_START_Y; // 重置動畫位置
            hand[i].targetY = HAND_START_Y;
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
        float x = HAND_START_X + i * CARD_STEP;
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
                hand[i].targetY = hand[i].selected ? (HAND_START_Y - 30) : HAND_START_Y;
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
// 排列順序：點數小 -> 點數大
int CompareCardsByRank(const void* a, const void* b)
{
    const Card* cardA = (const Card*)a;
    const Card* cardB = (const Card*)b;
    return cardA->rank - cardB->rank;
}

// 2. 核心函式：檢查牌型並計分
void CheckAndScoreHand(Card* deck, Card* hand, int handSize, int* deckTopIndex, float* score)
{
    // --- A. 收集玩家選中的牌 ---
    Card selectedCards[5]; // 最多選5張
    int selectedIndices[5]; // 記錄手牌的位置，方便之後標記 played
    int count = 0;

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

    if (count == 0) return; // 沒選牌就什麼都不做

    // --- B. 排序選中的牌 (重要！) ---
    // 這會把選中的牌依照點數 (Rank) 從小排到大，方便判斷對子或順子
    qsort(selectedCards, count, sizeof(Card), CompareCardsByRank);

    // --- C. 牌型判斷邏輯 ---
    bool isValidHand = false;
    float handScore = 0.0f;
    const char* handName = "";

    // 1. 判斷單張 (Single) - 1分
    if (count == 1)
    {
        isValidHand = true;
        handScore = 1.0f;
        handName = "Single";
    }
    // 2. 判斷對子 (Pair) - 2分
    else if (count == 2)
    {
        // 檢查兩張牌點數是否相同
        if (selectedCards[0].rank == selectedCards[1].rank)
        {
            isValidHand = true;
            handScore = 2.0f;
            handName = "Pair";
        }
    }

    // --- D. 結算 (若牌型合法) ---
    if (isValidHand)
    {
        printf("打出牌型: %s | 得分: %.1f\n", handName, handScore);
        
        // 加分
        *score += handScore;

        // 將手牌標記為已打出 (Played)
        for (int i = 0; i < count; i++)
        {
            int handIdx = selectedIndices[i];
            hand[handIdx].played = true;      // 標記打出
            hand[handIdx].selected = false;   // 取消選取
            hand[handIdx].currentY = HAND_START_Y; // 重置動畫位置
        }

        // 立即補牌
        DrawCards(deck, hand, handSize, deckTopIndex);
    }
    else
    {
        printf("無效牌型! (選了 %d 張)\n", count);
        for (int i = 0; i < handSize; i++)
        {
            hand[i].selected = false;
            hand[i].targetY = HAND_START_Y; // 卡牌縮回去
        }
    }
}