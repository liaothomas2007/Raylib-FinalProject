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
    

    while (!WindowShouldClose()) 
    {
        // *** 補牌邏輯：每幀都嘗試補牌 (如果打了牌，會在下一幀補上) ***
        DrawCards(deck,hand,7,&deckTopIndex); 

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        ShowDeck(); // 繪製牌堆
        UpdateAndDrawHand(hand, 7); // 繪製手牌與互動
        
        DrawText(TextFormat("Deck: %d", 52 - deckTopIndex), 10, 10, 20, BLACK);
        EndDrawing();
    }

    // 釋放記憶體
    UnloadTexture(card_back);
    // 這裡應也 Unload 所有 cardTextures
    free(deck);
    free(hand);

    CloseWindow();
    return 0;
}