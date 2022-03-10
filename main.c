#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "raylib.h"

int main() 
{
    // Initialization
    //--------------------------------------------------------------------------------------
    // constants
    const unsigned int SCORE_INCREMENT = 750;
    const unsigned int MAX_HINT_TIME = 60 * 30;
    const float BASE_COLOR_VEL = 24.0;
    const float ESCAPE_SPEED = 160.0;
    const float NORMAL_SPEED = 16.0;

    // color data
    Color colorPos = BLANK;
    colorPos.r = 255.0;
    float colorVel = -24.0;
    
    // hint data
    Color hintPos = BLACK;
    hintPos.a = 0.0;
    unsigned int hintTime = 0;

    // window data
    int currentMonitor = 0;
    int windowWidth = 256;
    int windowHeight = 256;
    Vector2 windowBoundsLower = {0, 0};
    Vector2 windowBoundsUpper = {0, 0};
    Vector2 windowPos = {0.0, 0.0};
    Vector2 windowDir = {0.0, 0.0};
    float windowVel = 0.0;

    // score tracking
    char* scoreText = calloc(256, 1);
    unsigned int score = 0;

    // debug
    char* winText = calloc(256, 1);
    char* posText = calloc(64, 1);
    char* velText = calloc(64, 1);

    // window setup
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_TRANSPARENT);
    InitWindow(windowWidth, windowHeight, "Score: 0");
    SetTargetFPS(60);
    SetWindowSize(windowWidth, windowHeight);

    // position setup
    windowPos.x = (GetMonitorWidth(currentMonitor) - windowWidth) / 2.0;
    windowPos.y = (GetMonitorHeight(currentMonitor) - windowHeight) / 2.0;
    SetWindowPosition(windowPos.x, windowPos.y);

    // velocity setup
    windowDir.x = (float)(rand()) / (float)(RAND_MAX);
    windowDir.y = (float)(rand()) / (float)(RAND_MAX);
    windowVel = NORMAL_SPEED;

    //--------------------------------------------------------------------------------------

    // Main game loop (close on window close button or ESC key)
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        
        // score decay step
        if (score > 0) { score--; }
        
        // halo step
        Vector2 mousePos = GetMousePosition();
        if (colorPos.a == 0.0
         && IsMouseButtonDown(MOUSE_LEFT_BUTTON)
         && mousePos.x > 0
         && mousePos.y > 0
         && mousePos.x < windowWidth
         && mousePos.y < windowHeight
        ) {
            // award score for catching the target
            score += SCORE_INCREMENT;
            // reset the hint timer
            hintTime = 0;
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
                windowVel = NORMAL_SPEED + ((ESCAPE_SPEED - NORMAL_SPEED) * (colorVel / BASE_COLOR_VEL));
            }
        } else {
            // increase hint timer gradually during normal movement
            if (hintTime < MAX_HINT_TIME) { hintTime++; }
        }
        
        // update hint color
        hintPos.a = 255.0 * ((1.0 * hintTime) / (1.0 * MAX_HINT_TIME));
        
        // update score display text
        sprintf(scoreText, "Score: %u", score);
        SetWindowTitle(scoreText);
        
        // window position and enclosure (current monitor)
        //windowPos = GetWindowPosition();
        currentMonitor = GetCurrentMonitor();
        windowBoundsLower.x = GetMonitorPosition(currentMonitor).x;
        windowBoundsLower.y = GetMonitorPosition(currentMonitor).y;
        windowBoundsUpper.x = windowBoundsLower.x + GetMonitorWidth(currentMonitor) - windowWidth;
        windowBoundsUpper.y = windowBoundsLower.y + GetMonitorHeight(currentMonitor) - windowHeight;
        // window collision step
        if (windowPos.x < windowBoundsLower.x && windowDir.x < 0.0) { windowDir.x *= -1; }
        if (windowPos.x > windowBoundsUpper.x && windowDir.x > 0.0) { windowDir.x *= -1; }
        if (windowPos.y < windowBoundsLower.y && windowDir.y < 0.0) { windowDir.y *= -1; }
        if (windowPos.y > windowBoundsUpper.y && windowDir.y > 0.0) { windowDir.y *= -1; }
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

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();
    free(winText);
    free(posText);
    free(velText);
    //--------------------------------------------------------------------------------------

    return 0;
}
