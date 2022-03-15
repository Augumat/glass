#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#include "resource.h"
#include "resloader.h"




// define struct for representing distinct tiers
typedef struct Tier {
    Image* displayImage;
    char* displayIconKey;
    char* displayNumeral;
    float haloSlope;
    float haloBase;
    float normalSpeed;
    float escapeSpeed;
    unsigned int minScore;
    unsigned int maxScore;
    unsigned int hitValue;
    unsigned int perfectHitValue;
    unsigned int penaltyTime;
    unsigned int penaltyFrequency;
    unsigned int penaltyIncrement;
} Tier;
//static void FreeTier(Tier* x) {
//    RL_FREE(*(x.displayImage));
//    free(x.displayIconKey);
//    free(x.displayNumeral);
//}



int main() 
{
    // Initialization
    //--------------------------------------------------------------------------------------
    // constants
    const unsigned int SCORE_INCREMENT = 1000;
    const unsigned int T1_SCORE_PENALTY = 500;
    const unsigned int T1_PENALTY_THRESHOLD = 5000;
    const unsigned int T2_SCORE_PENALTY = 250;
    const unsigned int T2_PENALTY_THRESHOLD = 10000;
    const unsigned int T3_SCORE_PENALTY = 125;
    const unsigned int T3_PENALTY_THRESHOLD = 25000;
    const unsigned int T4_SCORE_PENALTY = 75;
    const unsigned int T4_PENALTY_THRESHOLD = 50000;
    const unsigned int T5_SCORE_PENALTY = 50;
    const unsigned int T5_PENALTY_THRESHOLD = 1000000;
    const unsigned int MAX_HINT_TIME = 60 * 30;
    const float BASE_COLOR_VEL = 24.0;
    const float ESCAPE_SPEED = 160.0;
    const float NORMAL_SPEED = 16.0;

    // color data
    Color colorPos = RED;
    colorPos.a = 0.0;
    float colorVel = -24.0;
    
    // hint data
    Color hintPos = BLACK;
    unsigned int hintTime = MAX_HINT_TIME;
    unsigned char hasGameStarted = 0;

    // window data
    int currentMonitor = 0;
    int windowWidth = 200;
    int windowHeight = 200;
    Vector2 windowBoundsLower = {0, 0};
    Vector2 windowBoundsUpper = {0, 0};
    Vector2 windowPos = {0.0, 0.0};
    Vector2 windowDir = {0.0, 0.0};
    float windowVel = 0.0;

    // score tracking
    char* scoreText = calloc(256, 1);
    unsigned int score = 0;
    unsigned int highestScore = 0;
    unsigned int combo = 0;
    unsigned int highestCombo = 0;
    unsigned char highestTier = 0;
    
    // debug
    char* winText = calloc(256, 1);
    char* posText = calloc(64, 1);
    char* velText = calloc(64, 1);
    
    // temporary vars for use in packaged resource decoding
    int tempSize = 0;
    unsigned long tempSizeLong = 0;
    unsigned char* tempDecoded = NULL;
    
    // window setup
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_TRANSPARENT);
    InitWindow(windowWidth, windowHeight, "Score: 0");
    SetTargetFPS(60);
    SetWindowSize(windowWidth, windowHeight);
    // grab window icon
    tempDecoded = LoadPackagedResource(PNG_RESOURCE, TIER0_IMG, &tempSizeLong);
    Image windowIcon = LoadImageFromMemory(".png", tempDecoded, (int)(tempSizeLong));
    tempDecoded = NULL;
    SetWindowIcon(windowIcon);
    // set window starting position
    windowPos.x = (GetMonitorWidth(currentMonitor)  - windowWidth ) / 2.0;
    windowPos.y = (GetMonitorHeight(currentMonitor) - windowHeight) / 2.0;
    SetWindowPosition(windowPos.x, windowPos.y);

    // initialize the audio engine
    InitAudioDevice();
    while (!IsAudioDeviceReady()) {}
    // load sounds
    tempDecoded = LoadPackagedResource(WAV_RESOURCE, BOUNCE_SOUND, &tempSizeLong);
    Wave wBounce = LoadWaveFromMemory(".wav", tempDecoded, (int)(tempSizeLong));
    Sound sBounce = LoadSoundFromWave(wBounce);
    tempDecoded = NULL;
    tempDecoded = LoadPackagedResource(WAV_RESOURCE, HIT_SOUND, &tempSizeLong);
    Wave wHit = LoadWaveFromMemory(".wav", tempDecoded, (int)(tempSizeLong));
    Sound sHit = LoadSoundFromWave(wHit);
    tempDecoded = NULL;
    tempDecoded = LoadPackagedResource(WAV_RESOURCE, GAME_OVER_SOUND, &tempSizeLong);
    Wave wGameOver = LoadWaveFromMemory(".wav", tempDecoded, (int)(tempSizeLong));
    Sound sGameOver = LoadSoundFromWave(wGameOver);
    tempDecoded = NULL;
    
    // load save data
    //todo

    //--------------------------------------------------------------------------------------

    // Main game loop (close on window close button or ESC key)
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        
        // score decay step
        if (hasGameStarted && score > 0) {
            if (score > highestScore) { highestScore = score; }
            if (colorPos.a == 0.0) { score--; }
        }
        
        // halo step
        Vector2 mousePos = GetMousePosition();
        if (colorPos.a == 0.0
         && IsMouseButtonDown(MOUSE_LEFT_BUTTON)
         && mousePos.x > 0
         && mousePos.y > 0
         && mousePos.x < windowWidth
         && mousePos.y < windowHeight
        ) {
            // start the game if it hadn't already started
            hasGameStarted = 1;
            // award score for catching the target
            if (score < T1_PENALTY_THRESHOLD) {
                score += SCORE_INCREMENT
                       - (unsigned int)(T1_SCORE_PENALTY * ( (float)(score) / (float)(T1_PENALTY_THRESHOLD) ));
            } else if (score < T2_PENALTY_THRESHOLD) {
                score += SCORE_INCREMENT - T1_SCORE_PENALTY
                       - (unsigned int)(T2_SCORE_PENALTY * ( (float)(score - T1_PENALTY_THRESHOLD) / (float)(T2_PENALTY_THRESHOLD - T1_PENALTY_THRESHOLD) ));
            } else if (score < T3_PENALTY_THRESHOLD) {
                score += SCORE_INCREMENT - T1_SCORE_PENALTY - T2_SCORE_PENALTY
                       - (unsigned int)(T3_SCORE_PENALTY * ( (float)(score - T2_PENALTY_THRESHOLD) / (float)(T3_PENALTY_THRESHOLD - T2_PENALTY_THRESHOLD) ));
            } else if (score < T4_PENALTY_THRESHOLD) {
                score += SCORE_INCREMENT - T1_SCORE_PENALTY - T2_SCORE_PENALTY - T3_SCORE_PENALTY
                       - (unsigned int)(T4_SCORE_PENALTY * ( (float)(score - T3_PENALTY_THRESHOLD) / (float)(T4_PENALTY_THRESHOLD - T3_PENALTY_THRESHOLD) ));
            } else if (score < T5_PENALTY_THRESHOLD) {
                score += SCORE_INCREMENT - T1_SCORE_PENALTY - T2_SCORE_PENALTY - T3_SCORE_PENALTY
                       - (unsigned int)(T5_SCORE_PENALTY * ( (float)(score - T4_PENALTY_THRESHOLD) / (float)(T5_PENALTY_THRESHOLD - T4_PENALTY_THRESHOLD) ));
            } else {
                // reward for effectively impossible challenge
                //todo
            }
            // play the hurt sound (or the perfect hit sound if the player caught the pane on the first frame)
            if (hintTime == 0) {
                combo++;
                if (combo > highestCombo) { highestCombo = combo; }
                if (combo < 4) {
                    SetSoundPitch(sHit, 1.0 + 0.05 * combo);
                } else if (combo == 4) {
                    SetSoundPitch(sHit, 1.4);
                }
            } else {
                hintTime = 0;
                combo = 0;
                SetSoundPitch(sHit, 1.0);
            }
            // play the hit sound
            PlaySound(sHit);
            // start the halo animation
            colorPos.a = 255.0;
            colorVel = BASE_COLOR_VEL;
            // give the target a random escape path
            windowDir.x = (float)(rand()) / (float)(RAND_MAX);
            windowDir.y = (float)(rand()) / (float)(RAND_MAX);
            windowVel = ESCAPE_SPEED;
        } else if (colorPos.a > 0.0) {
            if (colorPos.a - colorVel < 0.0) {
                // halo is over, go back to normal speed
                colorPos.a = 0.0;
                windowVel = NORMAL_SPEED;
            } else {
                // smooth the damage animation
                colorPos.a -= colorVel;
                colorVel = (colorVel / 1.5) + (4.0 * (1.0 - (colorVel / BASE_COLOR_VEL)));
                // smooth the escape speed
                windowVel = NORMAL_SPEED + ((ESCAPE_SPEED - NORMAL_SPEED) * (colorPos.a / 255.0));
            }
        } else {
            // increase hint timer gradually during normal movement
            if (hintTime < MAX_HINT_TIME) { hintTime++; }
        }
        
        // update hint color
        hintPos.a = 255.0 * ((1.0 * hintTime) / (1.0 * MAX_HINT_TIME));
        
        // update score display text
        if (score < T1_PENALTY_THRESHOLD) {
            sprintf(scoreText, "< %u >", score);
        } else if (score < T2_PENALTY_THRESHOLD) {
            sprintf(scoreText, "{ %u }", score);
        } else if (score < T3_PENALTY_THRESHOLD) {
            sprintf(scoreText, "( %u )", score);
        } else if (score < T4_PENALTY_THRESHOLD) {
            sprintf(scoreText, "[ %u ]", score);
        } else if (score < T5_PENALTY_THRESHOLD) {
            sprintf(scoreText, "| %u |", score);
        } else {
            sprintf(scoreText, "%c %u %c", 64 + (rand() % 62), score, 64 + (rand() % 62));
        }
        SetWindowTitle(scoreText);
        
        // window position and enclosure (current monitor)
        currentMonitor = GetCurrentMonitor();
        windowBoundsLower.x = GetMonitorPosition(currentMonitor).x;
        windowBoundsLower.y = GetMonitorPosition(currentMonitor).y;
        windowBoundsUpper.x = windowBoundsLower.x + GetMonitorWidth(currentMonitor) - windowWidth;
        windowBoundsUpper.y = windowBoundsLower.y + GetMonitorHeight(currentMonitor) - windowHeight;
        // window collision step
        if (windowPos.x < windowBoundsLower.x && windowDir.x < 0.0) { windowDir.x *= -1; PlaySound(sBounce); }
        if (windowPos.x > windowBoundsUpper.x && windowDir.x > 0.0) { windowDir.x *= -1; PlaySound(sBounce); }
        if (windowPos.y < windowBoundsLower.y && windowDir.y < 0.0) { windowDir.y *= -1; PlaySound(sBounce); }
        if (windowPos.y > windowBoundsUpper.y && windowDir.y > 0.0) { windowDir.y *= -1; PlaySound(sBounce); }
        // window movement step
        windowPos.x += windowDir.x * windowVel;
        windowPos.y += windowDir.y * windowVel;
        SetWindowPosition(windowPos.x, windowPos.y);

        // update info text
        sprintf(
            winText, "%d: %s\n(%d, %d)\n(%.2f, %.2f)\n(%.2f, %.2f)",
            currentMonitor, GetMonitorName(currentMonitor),
            GetMonitorWidth(currentMonitor), GetMonitorHeight(currentMonitor),
            windowBoundsLower.x, windowBoundsUpper.x,
            windowBoundsLower.y, windowBoundsUpper.y
        );
        sprintf(posText, "(%.0f, %.0f)", windowPos.x, windowPos.y);
        sprintf(velText, "(%.2f, %.2f) @ (%.2f)", windowDir.x, windowDir.y, windowVel);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            
            // refresh frame
            ClearBackground(BLANK);
            // draw halo rect
            DrawRectangle(0, 0, windowWidth, windowHeight, colorPos);
            // draw hint rect
            DrawRectangle(0, 0, windowWidth, windowHeight, hintPos);

            // debug info
            if (IsKeyDown(KEY_LEFT_BRACKET) && IsKeyDown(KEY_RIGHT_BRACKET)) {
                if (colorPos.a == 0.0) {
                    DrawRectangle(0, 0, windowWidth, windowHeight, WHITE);
                }
                if (IsKeyDown(KEY_BACKSLASH)) {
                    DrawText(winText, 10, 10,  20, BLACK);
                    DrawText(posText, 10, 130, 20, GRAY);
                    DrawText(velText, 10, 160, 20, GRAY);
                }
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    
    // Save high score to file
    //todo
    
    // Clean up window
    //--------------------------------------------------------------------------------------
    CloseWindow();
    free(winText);
    free(posText);
    free(velText);
    //--------------------------------------------------------------------------------------
    
    // Play game over sound and clean up audio
    //--------------------------------------------------------------------------------------
    
    // Play game over sound
    PlaySound(sGameOver);
    while (IsSoundPlaying(sGameOver)) {}
    
    // Unload sound effects
    UnloadWave(wBounce);
    UnloadSound(sBounce);
    UnloadWave(wHit);
    UnloadSound(sHit);
    UnloadWave(wGameOver);
    UnloadSound(sGameOver);
    
    // Close the audio context
    CloseAudioDevice();
    
    //--------------------------------------------------------------------------------------
    
    return 0;
}
