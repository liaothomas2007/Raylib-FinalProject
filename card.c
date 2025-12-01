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
const int HAND_START_X = 150;  
const int HAND_START_Y = 700;  
const int CARD_GAP = 10;      
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

// *** 修正/新增 ShowDeck 的定義 ***
void ShowDeck()
{
    const float scale = 0.35f;
    const int margin = 20;
    Vector2 pilePos = 
    {
        GetScreenWidth() - card_back.width * scale - margin,
        GetScreenHeight() - card_back.height * scale - margin
    };
    DrawTextureEx(card_back, pilePos, 0, scale, WHITE);
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

        // 定義碰撞框
        Rectangle rect = { x, y, CARD_WIDTH_BASE * CARD_SCALE, CARD_HEIGHT_BASE * CARD_SCALE};

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

    // 處理按下空白鍵打牌邏輯
    if (IsKeyPressed(KEY_SPACE))
    {
        // 模擬打出：牌型判斷功能實現前，先標記為 played=true
        for (int i = 0; i < handSize; i++)
        {
            if (hand[i].selected)
            {
                hand[i].played = true; // 標記為打出
                hand[i].selected = false; // 取消選取
            }
        }
        // 補牌邏輯會在 main.c 的迴圈中呼叫 DrawCards 處理
    }
}