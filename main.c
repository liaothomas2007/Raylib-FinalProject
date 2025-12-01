// main.c
//初始化 + 呼叫主流程
#include "raylib.h"
#include "card.h"
#include <stdlib.h>

int main() 
{
    InitWindow(1280, 900, "Raylib Card Demo");
    SetTargetFPS(60);

    Card* deck = (Card*)malloc(52 * sizeof(Card));
    Card* hand = (Card*)malloc(7 * sizeof(Card));

    // 初始化手牌狀態
    for(int i=0; i<7; i++) hand[i].played = true; // 設為 true 讓 DrawCards 第一次執行時自動補滿

    int deckTopIndex = 0; // 記錄發牌發到哪
    
    LoadCardTextures(); //載入圖片
    InitDeck(deck); //建立主牌組
    ShuffleDeck(deck,52);
    
    // 第一次發牌 (使用指標傳遞)
    DrawCards(deck,hand,7,&deckTopIndex);
    
    float score = 0.0f; // 1. 確保有宣告分數變數

    while (!WindowShouldClose()) 
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            CheckAndScoreHand(deck, hand, 7, &deckTopIndex, &score);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        ShowDeck();
        UpdateAndDrawHand(hand, 7);
        
        // 3. 顯示 UI
        DrawText(TextFormat("Deck: %d", 52 - deckTopIndex), 10, 10, 20, BLACK);
        DrawText(TextFormat("Score: %.1f", score), 10, 40, 20, DARKBLUE); // 顯示分數

        EndDrawing();
    }

    // 釋放記憶體
    UnloadTexture(card_back);

    for (int s = 0; s < 4; s++) {
        for (int r = 0; r < 13; r++) {
            UnloadTexture(cardTextures[s][r]);
        }
    }

    free(deck);
    free(hand);

    CloseWindow();
    return 0;
}