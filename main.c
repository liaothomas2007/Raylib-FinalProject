// main.c
//初始化 + 呼叫主流程
#include "raylib.h"
#include "card.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h> 

// --- 定義遊戲狀態 ---
typedef enum {
    GAME_MENU,
    GAME_PLAYING
} GameState;

// 定義關卡目標
const float LEVEL_TARGETS[3] = {55.0f, 60.0f, 65.0f};
#define MAX_HANDS 10

// 輔助函式：重置/開始關卡 (這是切換關卡時必須的)
void ResetLevel(Card* deck, Card* hand, int* deckTopIndex, float* score, int* handsPlayed) {
    *deckTopIndex = 0;
    *score = 0.0f;
    *handsPlayed = 0;
    
    InitDeck(deck);
    ShuffleDeck(deck, 52);
    // 重置手牌
    for(int i=0; i<7; i++) {
        hand[i].played = true; 
        hand[i].selected = false;
        // 確保手牌位置重置，避免視覺錯誤
        // 修正：使用常數 HAND_START_Y 保持一致性
        hand[i].currentY = (float)HAND_START_Y; 
        hand[i].targetY = (float)HAND_START_Y;
    }
    // 立即補滿手牌
    DrawCards(deck, hand, 7, deckTopIndex);
}

// 輔助函式：繪製按鈕 (讓主程式更乾淨)
bool DrawButton(const char* text, Rectangle rect, Font font) {
    Vector2 mousePoint = GetMousePosition();
    bool isHover = CheckCollisionPointRec(mousePoint, rect);
    
    // 繪製按鈕背景 (滑鼠懸停時變色)
    DrawRectangleRec(rect, isHover ? SKYBLUE : LIGHTGRAY);
    DrawRectangleLinesEx(rect, 3, DARKGRAY);
    
    // 繪製文字 (置中)
    // 這裡使用 MeasureText (預設字型) 或 MeasureTextEx (自訂字型)
    int textWidth = MeasureText(text, 20);
    DrawText(text, (int)(rect.x + rect.width/2 - textWidth/2), (int)(rect.y + rect.height/2 - 10), 20, BLACK);

    return (isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

int main() 
{   
    InitWindow(1280, 900, "Raylib Card Demo");
    SetTargetFPS(60);

    // 修正：宣告一個預設字型，避免 DrawButton 報錯
    Font font = GetFontDefault();

    Card* deck = (Card*)malloc(52 * sizeof(Card));
    Card* hand = (Card*)malloc(7 * sizeof(Card));

    // --- 初始化遊戲修改器 (預設值) ---
    GameModifiers mods = {0}; 
    mods.multSingle = 1.0f; // 單張預設 1倍
    mods.multPair = 1.0f;   // 對子預設 1倍
    mods.bonusChips = 0;    // 無額外籌碼

    // 初始化手牌狀態
    for(int i=0; i<7; i++) hand[i].played = true; 

    int deckTopIndex = 0; // 記錄發牌發到哪
    
    LoadCardTextures(); //載入圖片
    InitDeck(deck); //建立主牌組
    ShuffleDeck(deck,52);
    
    // 第一次發牌 (使用指標傳遞)
    DrawCards(deck,hand,7,&deckTopIndex);
    
    // --- 遊戲變數初始化 ---
    GameState currentState = GAME_MENU; // 預設從選單開始
    int currentLevel = 1;
    
    // 錯誤修正：這裡原本重複宣告了 int deckTopIndex，已移除
    
    float score = 0.0f;
    int handsPlayed = 0;
    
    bool isLevelClear = false;
    bool isGameClear = false;
    bool isGameOver = false;
    char message[100] = "";

    while (!WindowShouldClose()) 
    {
       if (currentState == GAME_PLAYING)
        {
            float targetScore = LEVEL_TARGETS[currentLevel - 1];
            DrawText(TextFormat("Single Mult: x%.1f", mods.multSingle), 10, 130, 20, PURPLE);
            if (!isGameOver && !isGameClear && !isLevelClear)
            {
                if (IsKeyPressed(KEY_SPACE))
                {
                    float oldScore = score;
                    // 注意：記得要確認您的 card.h 已經加入 level 參數
                    CheckAndScoreHand(deck, hand, 7, &deckTopIndex, &score, currentLevel, &mods);
                    
                    if (score > oldScore) {
                        handsPlayed++;
                        sprintf(message, "Score! +%.1f", score - oldScore);
                    } else {
                        sprintf(message, "Invalid Hand!");
                    }
                }
                
                // 檢查勝負
                if (score >= targetScore) isLevelClear = true;
                else if (handsPlayed >= MAX_HANDS) isGameOver = true;
            }

            // 處理過關進入下一關
            if (isLevelClear && IsKeyPressed(KEY_ENTER)) {
                if (currentLevel < 3) {
                    currentLevel++;
                    isLevelClear = false;
                    ResetLevel(deck, hand, &deckTopIndex, &score, &handsPlayed);
                } else {
                    isGameClear = true;
                }
            }

            // 隨時可以按 ESC 回到選單 (或是失敗/全破後按 R)
            if (IsKeyPressed(KEY_ESCAPE) || (isGameOver && IsKeyPressed(KEY_R)) || (isGameClear && IsKeyPressed(KEY_R)))
            {
                currentState = GAME_MENU; // 回到選單
                isGameOver = false;
                isGameClear = false;
                isLevelClear = false;
                // 回選單建議也重置一下變數，或者保持狀態
            }
        } 

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        if (currentState == GAME_MENU)
        {
            // --- 繪製開始選單 ---
            DrawText("GAME MENU", 500, 100, 60, DARKBLUE);
            DrawText("Select a Level to Start", 480, 180, 30, DARKGRAY);

            // 繪製三個關卡按鈕
            if (DrawButton("Level 1 (Target: 55)", (Rectangle){440, 300, 400, 80}, font)) {
                currentLevel = 1;
                ResetLevel(deck, hand, &deckTopIndex, &score, &handsPlayed);
                currentState = GAME_PLAYING;
            }
            
            if (DrawButton("Level 2 (Target: 60)", (Rectangle){440, 400, 400, 80}, font)) {
                currentLevel = 2;
                ResetLevel(deck, hand, &deckTopIndex, &score, &handsPlayed);
                currentState = GAME_PLAYING;
            }

            if (DrawButton("Level 3 (Target: 65)", (Rectangle){440, 500, 400, 80}, font)) {
                currentLevel = 3;
                ResetLevel(deck, hand, &deckTopIndex, &score, &handsPlayed);
                currentState = GAME_PLAYING;
            }
        }
        else if (currentState == GAME_PLAYING)
        {
            // --- 繪製遊戲畫面 ---
            UpdateAndDrawHand(hand, 7);
            
            // UI 顯示
            DrawText(TextFormat("Level: %d", currentLevel), 10, 10, 30, BLUE);
            DrawText(TextFormat("Score: %.1f / %.0f", score, LEVEL_TARGETS[currentLevel-1]), 10, 50, 30, DARKGREEN);
            DrawText(TextFormat("Hands: %d / %d", handsPlayed, MAX_HANDS), 10, 90, 30, BLACK);
            DrawText(message, 400, 50, 30, RED);
            
            // 提示按鍵
            DrawText("[ESC] Back to Menu", 1000, 10, 20, GRAY);

            // 狀態視窗 (過關/失敗)
            if (isLevelClear && !isGameClear) {
                DrawRectangle(300, 200, 680, 300, Fade(WHITE, 0.9f));
                DrawRectangleLines(300, 200, 680, 300, BLACK);
                DrawText("LEVEL CLEARED!", 450, 250, 50, GREEN);
                DrawText("Press [ENTER] for Next Level", 380, 350, 40, DARKGRAY);
            }
            else if (isGameClear) {
                DrawRectangle(300, 200, 680, 300, Fade(GOLD, 0.9f));
                DrawText("CONGRATULATIONS!", 380, 250, 50, MAROON);
                DrawText("Press [R] to Return Menu", 400, 400, 30, DARKGRAY);
            }
            else if (isGameOver) {
                DrawRectangle(300, 200, 680, 300, Fade(BLACK, 0.8f));
                DrawText("GAME OVER", 500, 250, 50, RED);
                DrawText("Press [R] to Return Menu", 400, 350, 30, WHITE);
            }
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