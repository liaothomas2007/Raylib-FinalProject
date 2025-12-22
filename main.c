// main.c
//初始化 + 呼叫主流程
#include "card.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h> 
#include <raylib.h>

// --- 定義遊戲狀態 ---
typedef enum {
    GAME_MENU,
    GAME_PLAYING,
    GAME_MAGIC_SELECT
} GameState;

// 定義關卡目標
const float LEVEL_TARGETS[3] = {55.0f, 60.0f, 65.0f};
#define MAX_HANDS 10

// --- 魔法卡資料庫 (可以自己擴充) ---
const MagicCard CARD_DATABASE[] = {
    {"Single Master", "Single Hand Mult +1.0", EFFECT_MULT_SINGLE, 1.0f, SKYBLUE},
    {"Pair Expert",   "Pair Hand Mult +1.0",   EFFECT_MULT_PAIR,   1.0f, ORANGE},
    {"Chip Bonus",    "All Hands +10 Chips",   EFFECT_BONUS_CHIPS, 10.0f, GOLD},
    {"Single Boost",  "Single Hand Mult +2.0", EFFECT_MULT_SINGLE, 2.0f, DARKBLUE},
    {"Mega Chips",    "All Hands +20 Chips",   EFFECT_BONUS_CHIPS, 20.0f, YELLOW}
};
const int TOTAL_MAGIC_CARDS = 5;

void GenerateMagicOptions(MagicCard options[]) {
    for (int i = 0; i < 3; i++) {
        int randIdx = rand() % TOTAL_MAGIC_CARDS;
        options[i] = CARD_DATABASE[randIdx];
    }
}

// 繪製魔法卡 UI
bool DrawMagicCard(MagicCard card, Rectangle rect) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);
    
    // 背景
    DrawRectangleRec(rect, hover ? Fade(card.color, 0.8f) : card.color);
    DrawRectangleLinesEx(rect, 3, BLACK);
    
    // 文字
    DrawText(card.name, (int)(rect.x + 10), (int)(rect.y + 20), 30, BLACK);
    DrawText(card.description, (int)(rect.x + 10), (int)(rect.y + 60), 20, DARKGRAY);
    DrawText("CLICK TO SELECT", (int)(rect.x + 10), (int)(rect.y + 160), 15, WHITE);
    
    return (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

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
    mods.multStraight = 1.0f;
    mods.multFlush = 1.0f;
    mods.multFullHouse = 1.0f;
    mods.multFourOfAKind = 1.0f;
    mods.multStraightFlush = 1.0f;
    mods.bonusChips = 0;

    MagicCard currentOptions[3];
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
                    GenerateMagicOptions(currentOptions);
                    // 2. 切換狀態
                    currentState = GAME_MAGIC_SELECT;
                    isLevelClear = false;
                } else {
                    isGameClear = true;
                }
            }

            if ((isGameOver || isGameClear) && IsKeyPressed(KEY_R)) {
                currentState = GAME_MENU;
                isGameOver = false;
                isGameClear = false;
                mods = (GameModifiers){1.0f, 1.0f, 0}; // 重置能力
            }
  
        } 

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        if (currentState == GAME_MENU) {
            DrawText("GAME MENU", 500, 100, 60, BLUE);
            if (DrawButton("Start Level 1", (Rectangle){500, 300, 280, 60},font)) {
                currentLevel = 1;
                ResetLevel(deck, hand, &deckTopIndex, &score, &handsPlayed);
                currentState = GAME_PLAYING;
            }
        }
        else if (currentState == GAME_PLAYING) {
            UpdateAndDrawHand(hand, 7);
            DrawText(TextFormat("Single Mult: x%.1f", mods.multSingle), 10, 130, 20, PURPLE);
            DrawText(TextFormat("Level: %d", currentLevel), 10, 10, 30, BLUE);
            DrawText(TextFormat("Score: %.1f / %.0f", score, LEVEL_TARGETS[currentLevel-1]), 10, 50, 30, DARKGREEN);
            DrawText(TextFormat("Hands: %d / %d", handsPlayed, MAX_HANDS), 10, 90, 30, BLACK);
            
            // 顯示當前能力
            DrawText(TextFormat("Mods: Single x%.1f | Pair x%.1f | Bonus +%d", 
                     mods.multSingle, mods.multPair, mods.bonusChips), 300, 10, 20, PURPLE);

            if (isLevelClear) {
                 DrawRectangle(300, 200, 680, 300, Fade(WHITE, 0.9f));
                 DrawRectangleLines(300, 200, 680, 300, BLACK);
                 DrawText("LEVEL CLEARED!", 450, 250, 50, GREEN);
                 DrawText("Press [ENTER] to Select Magic Card", 350, 350, 30, DARKGRAY);
            }
            // ... (GameOver / GameClear 顯示邏輯)
        }
        else if (currentState == GAME_MAGIC_SELECT) {
            // --- 魔法卡選擇介面 ---
            DrawText("Select a Magic Card!", 450, 100, 40, DARKBLUE);
            
            // 繪製三張卡
            for (int i = 0; i < 3; i++) {
                Rectangle rect = {300 + i * 250, 250, 220, 300};
                if (DrawMagicCard(currentOptions[i], rect)) {
                    // --- 點擊後的邏輯 ---
                    MagicCard chosen = currentOptions[i];
                    
                    // 應用效果
                    if (chosen.type == EFFECT_MULT_SINGLE) mods.multSingle += chosen.value;
                    if (chosen.type == EFFECT_MULT_PAIR)   mods.multPair += chosen.value;
                    if (chosen.type == EFFECT_BONUS_CHIPS) mods.bonusChips += (int)chosen.value;
                    
                    // 進入下一關
                    currentLevel++;
                    ResetLevel(deck, hand, &deckTopIndex, &score, &handsPlayed);
                    currentState = GAME_PLAYING;
                }
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