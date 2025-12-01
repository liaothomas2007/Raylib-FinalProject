// main.c
//初始化 + 呼叫主流程
#include "raylib.h"
#include "card.h"
#include <stdlib.h>
#include <stdbool.h> // 確保可以使用 bool

#define TARGET_SCORE 20.0f  // 預設目標分數
#define MAX_HANDS 10        // 最大出牌次數

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
    
    // --- 遊戲狀態變數初始化 ---
    float score = 0.0f;         // 目前分數
    int handsPlayed = 0;        // 成功出牌次數 (手數)
    bool isGameOver = false;    // 遊戲是否結束

    while (!WindowShouldClose()) 
    {
       if (!isGameOver && IsKeyPressed(KEY_SPACE))
        {
            float oldScore = score;
            
            // 嘗試出牌並計分
            CheckAndScoreHand(deck, hand, 7, &deckTopIndex, &score);
            
            // 如果分數有增加，表示打牌成功
            if (score > oldScore) {
                handsPlayed++;
            }
        }

        if (!isGameOver) {
            if (score >= TARGET_SCORE) {
                isGameOver = true; // 勝利條件
            } else if (handsPlayed >= MAX_HANDS) {
                isGameOver = true; // 失敗條件 (手數用完)
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        UpdateAndDrawHand(hand, 7);
        
        // 3. 顯示 UI
        DrawText(TextFormat("Deck: %d", 52 - deckTopIndex), 10, 10, 20, BLACK);
        DrawText(TextFormat("Score: %.1f", score), 10, 40, 20, DARKBLUE); // 顯示分數 
        Color handsColor = (handsPlayed >= MAX_HANDS && score < TARGET_SCORE) ? RED : BLACK;
        DrawText(TextFormat("已出牌手數: %d / %d", handsPlayed, MAX_HANDS), 10, 75, 25, handsColor);
        
        // --- 顯示遊戲結果 (中央) ---
        if (isGameOver)
        {
            const char* resultText;
            Color resultColor;
            
            if (score >= TARGET_SCORE) {
                resultText = TextFormat("勝利! 達成目標共用 %d 手", handsPlayed);
                resultColor = GREEN;
            } else {
                resultText = TextFormat("失敗! %d 手內分數不足 (%.1f)", MAX_HANDS, score);
                resultColor = RED;
            }
            
            int screenWidth = GetScreenWidth();
            // 繪製大的結果文字在畫面中間
            DrawText(resultText, screenWidth/2 - MeasureText(resultText, 50)/2, 300, 50, resultColor);
        }
        
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