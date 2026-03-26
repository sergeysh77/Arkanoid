#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "arkanoid.h"
#include "resources.h"

// Global variable
GameState g_game;

// Internal function prototypes
static void ResetGameState(void);
static void UpdateBall(float deltaTime);
static void HandlePaddleCollision(void);
static void HandleBrickCollision(Brick* brick, int i, int j);
static void CheckBallCollisions(void);
static void LaunchBall(void);
static void LoseLife(void);
static void DrawPlaying(HDC memDC);

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = NULL;
    wc.lpszClassName = "ArkanoidClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(APP_ICON));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(APP_ICON));
    
    RegisterClassExA(&wc);
    
    g_game.hwnd = CreateWindowExA(
        0, "ArkanoidClass", "Arkanoid",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_game.hwnd) return 0;
    
    HICON hIconBig = LoadIcon(hInstance, MAKEINTRESOURCE(APP_ICON));
    HICON hIconSmall = LoadIcon(hInstance, MAKEINTRESOURCE(APP_ICON));
    SendMessage(g_game.hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
    SendMessage(g_game.hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    
    ShowWindow(g_game.hwnd, nCmdShow);
    UpdateWindow(g_game.hwnd);
    
    srand(GetTickCount());
    InitSound();
    InitLanguage();
    ResetGame();
    g_game.lastTime = GetTickCount();
    SetTimer(g_game.hwnd, TIMER_ID, TIMER_SPEED, NULL);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    KillTimer(g_game.hwnd, TIMER_ID);
    CleanupSound();
    return msg.wParam;
}

/*
=======================
Game Functions
=======================
*/

void LoadLevelData() {
    LoadLevel(g_game.currentLevel, g_game.bricks, BRICK_ROWS, BRICK_COLS, 
              BRICK_WIDTH, BRICK_HEIGHT, 50, &g_game.totalBricks);
}

static void ResetGameState() {
    g_game.paddle.width = g_game.paddle.originalWidth;
    g_game.paddle.x = SCREEN_WIDTH / 2 - g_game.paddle.width / 2;
    g_game.paddle.y = SCREEN_HEIGHT - 50;
    g_game.paddle.stickyMode = 0;
    
    g_game.ball.x = g_game.paddle.x + g_game.paddle.width / 2;
    g_game.ball.y = g_game.paddle.y - g_game.ball.radius;
    g_game.ball.active = 0;
    g_game.ball.sticky = 0;
    g_game.ball.hasBounced = 0;
    g_game.ball.stickedBall = 0;
    g_game.ball.stickedBallOffsetX = 0;
    
    g_game.ball.lastSpeedX = BALL_SPEED * 0.7f;
    g_game.ball.lastSpeedY = -BALL_SPEED * 0.7f;
    
    g_game.stickyTimerEndTime = 0;
    g_game.slowTimerEndTime = 0;
    
    float currentSpeed = sqrtf(g_game.ball.speedX * g_game.ball.speedX + 
                                g_game.ball.speedY * g_game.ball.speedY);
    if (currentSpeed != BALL_SPEED && currentSpeed > 0) {
        float ratio = BALL_SPEED / currentSpeed;
        g_game.ball.speedX *= ratio;
        g_game.ball.speedY *= ratio;
    }
    
    for (int i = 0; i < MAX_POWERUPS; i++) {
        g_game.powerups[i].active = 0;
    }
}

void ResetGame() {
    g_game.paddle.originalWidth = PADDLE_WIDTH;
    g_game.paddle.width = PADDLE_WIDTH;
    g_game.paddle.height = PADDLE_HEIGHT;
    g_game.paddle.lives = 3;
    
    g_game.ball.radius = BALL_SIZE;
    g_game.ball.speedX = BALL_SPEED * 0.7f;
    g_game.ball.speedY = -BALL_SPEED * 0.7f;
    
    ResetGameState();
    LoadLevelData();
    
    g_game.score = 0;
    g_game.currentLevel = 1;
    g_game.lastTime = GetTickCount();
}

void NextLevel() {
    g_game.currentLevel++;
    if (g_game.currentLevel > TOTAL_LEVELS) {
        g_game.gameState = GAME_VICTORY;
        return;
    }
    
    ResetGameState();
    LoadLevelData();
    InvalidateRect(g_game.hwnd, NULL, FALSE);
}


/*
=======================
Ball Logic
=======================
*/

static void UpdateBall(float deltaTime) {
    g_game.ball.x += g_game.ball.speedX * deltaTime;
    g_game.ball.y += g_game.ball.speedY * deltaTime;
    
    // Collision with left wall
    if (g_game.ball.x - g_game.ball.radius <= 0) {
        g_game.ball.x = g_game.ball.radius;
        g_game.ball.speedX = -g_game.ball.speedX;
        g_game.ball.hasBounced = 1;
    }
    // Collision with right wall
    if (g_game.ball.x + g_game.ball.radius >= SCREEN_WIDTH) {
        g_game.ball.x = SCREEN_WIDTH - g_game.ball.radius;
        g_game.ball.speedX = -g_game.ball.speedX;
        g_game.ball.hasBounced = 1;
    }
    // Collision with top wall
    if (g_game.ball.y - g_game.ball.radius <= 0) {
        g_game.ball.y = g_game.ball.radius;
        g_game.ball.speedY = -g_game.ball.speedY;
        g_game.ball.hasBounced = 1;
    }
}

static void HandlePaddleCollision(void) {
    // Adjust position
    g_game.ball.y = g_game.paddle.y - g_game.ball.radius;
    
    if (g_game.paddle.stickyMode) {
        // Calculate target speed
        float targetSpeed = BALL_SPEED;
        if (g_game.slowTimerEndTime > 0) {
            targetSpeed = BALL_SPEED * 0.5f;  // 150 with Slow powerup
        }
        
        // Normalize speed to targetSpeed
        float currentSpeed = sqrtf(g_game.ball.speedX * g_game.ball.speedX + 
                                    g_game.ball.speedY * g_game.ball.speedY);
        if (currentSpeed > 0.01f) {
            float ratio = targetSpeed / currentSpeed;
            g_game.ball.lastSpeedX = g_game.ball.speedX * ratio;
            g_game.ball.lastSpeedY = g_game.ball.speedY * ratio;
        } else {
            // If speed is almost zero, set standard
            g_game.ball.lastSpeedX = targetSpeed * 0.7f;
            g_game.ball.lastSpeedY = -targetSpeed * 0.7f;
        }
        
        // Remember sticky position
        g_game.ball.stickedBallOffsetX = g_game.ball.x - g_game.paddle.x;
        g_game.ball.active = 0;
        g_game.ball.sticky = 1;
        g_game.ball.stickedBall = 1;
    } else {
        g_game.ball.speedY = -g_game.ball.speedY;
        
        float hitPos = (g_game.ball.x - g_game.paddle.x) / g_game.paddle.width;
        float newSpeedX = (hitPos - 0.5f) * BALL_SPEED * MAX_BOUNCE_ANGLE;
        newSpeedX = max(-BALL_SPEED * MAX_BOUNCE_ANGLE, 
                        min(BALL_SPEED * MAX_BOUNCE_ANGLE, newSpeedX));
        
        float currentSpeed = sqrtf(g_game.ball.speedX * g_game.ball.speedX + 
                                    g_game.ball.speedY * g_game.ball.speedY);
        g_game.ball.speedX = newSpeedX;
        g_game.ball.speedY = -sqrtf(fmaxf(0.1f, currentSpeed * currentSpeed - 
                                          g_game.ball.speedX * g_game.ball.speedX));
        
        float finalSpeed = sqrtf(g_game.ball.speedX * g_game.ball.speedX + 
                                 g_game.ball.speedY * g_game.ball.speedY);
        
        if (g_game.slowTimerEndTime > 0) {
            float targetSpeed = BALL_SPEED * 0.5f;
            if (finalSpeed > targetSpeed) {
                float ratio = targetSpeed / finalSpeed;
                g_game.ball.speedX *= ratio;
                g_game.ball.speedY *= ratio;
                finalSpeed = targetSpeed;
            }
        }
        
        if (finalSpeed > BALL_SPEED * MAX_SPEED_FACTOR) {
            g_game.ball.speedX = g_game.ball.speedX * (BALL_SPEED * MAX_SPEED_FACTOR) / finalSpeed;
            g_game.ball.speedY = g_game.ball.speedY * (BALL_SPEED * MAX_SPEED_FACTOR) / finalSpeed;
        }
        if (finalSpeed < BALL_SPEED * MIN_SPEED_FACTOR && g_game.slowTimerEndTime == 0) {
            g_game.ball.speedX = g_game.ball.speedX * (BALL_SPEED * MIN_SPEED_FACTOR) / finalSpeed;
            g_game.ball.speedY = g_game.ball.speedY * (BALL_SPEED * MIN_SPEED_FACTOR) / finalSpeed;
        }
    }
}

static void HandleBrickCollision(Brick* brick, int i, int j) {
    PlaySoundEffect("hit_brick.wav");
    brick->hits++;
    g_game.ball.hasBounced = 1;
    
    if (brick->hits >= brick->maxHits) {
        brick->active = 0;
        g_game.totalBricks--;
        
        switch (brick->type) {
            case BRICK_NORMAL: g_game.score += 10; break;
            case BRICK_HARD: g_game.score += 20; break;
            case BRICK_STRONG: g_game.score += 30; break;
        }
        
        SpawnPowerup(brick->x, brick->y);
    } else {
        g_game.score += 5;
    }
    
    // Handle collisions
    float overlapLeft = (g_game.ball.x + g_game.ball.radius) - brick->x;
    float overlapRight = (brick->x + BRICK_WIDTH) - (g_game.ball.x - g_game.ball.radius);
    float overlapTop = (g_game.ball.y + g_game.ball.radius) - brick->y;
    float overlapBottom = (brick->y + BRICK_HEIGHT) - (g_game.ball.y - g_game.ball.radius);
    
    // Determine minimum overlap to choose bounce side
    float minOverlap = overlapLeft;
    int side = 0; // 0=left, 1=right, 2=top, 3=bottom
    
    if (overlapRight < minOverlap) { minOverlap = overlapRight; side = 1; }
    if (overlapTop < minOverlap) { minOverlap = overlapTop; side = 2; }
    if (overlapBottom < minOverlap) { minOverlap = overlapBottom; side = 3; }
    
    switch (side) {
        case 0: // Left side
            g_game.ball.speedX = -fabs(g_game.ball.speedX);
            break;
        case 1: // Right side
            g_game.ball.speedX = fabs(g_game.ball.speedX);
            break;
        case 2: // Top side
            g_game.ball.speedY = -fabs(g_game.ball.speedY);
            break;
        case 3: // Bottom side
            g_game.ball.speedY = fabs(g_game.ball.speedY);
            break;
    }
}

static void CheckBallCollisions(void) {
    // Collision with paddle
    if (g_game.ball.y + g_game.ball.radius >= g_game.paddle.y && 
        g_game.ball.y - g_game.ball.radius <= g_game.paddle.y + g_game.paddle.height &&
        g_game.ball.x + g_game.ball.radius >= g_game.paddle.x && 
        g_game.ball.x - g_game.ball.radius <= g_game.paddle.x + g_game.paddle.width) {
        
        // Play sound only if ball has already bounced and not in sticky mode
        if (g_game.ball.hasBounced && !g_game.paddle.stickyMode) {
            PlaySoundEffect("paddle_hit.wav");
        }
        
        HandlePaddleCollision();
        return;
    }
    
    // Collision with bricks
    for (int i = 0; i < BRICK_ROWS && g_game.totalBricks > 0; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (g_game.bricks[i][j].active) {
                RECT brickRect = {
                    (int)g_game.bricks[i][j].x,
                    (int)g_game.bricks[i][j].y,
                    (int)(g_game.bricks[i][j].x + BRICK_WIDTH),
                    (int)(g_game.bricks[i][j].y + BRICK_HEIGHT)
                };
                
                RECT ballRect = {
                    (int)(g_game.ball.x - g_game.ball.radius),
                    (int)(g_game.ball.y - g_game.ball.radius),
                    (int)(g_game.ball.x + g_game.ball.radius),
                    (int)(g_game.ball.y + g_game.ball.radius)
                };
                
                RECT intersect;
                if (IntersectRect(&intersect, &ballRect, &brickRect)) {
                    HandleBrickCollision(&g_game.bricks[i][j], i, j);
                    if (g_game.totalBricks == 0) NextLevel();
                    return;
                }
            }
        }
    }
    
    // Life lost
    if (g_game.ball.y + g_game.ball.radius >= SCREEN_HEIGHT) {
        LoseLife();
    }
}


static void LaunchBall(void) {
    g_game.ball.active = 1;
    g_game.ball.hasBounced = 0;
    
    if (g_game.ball.sticky) {
        g_game.ball.sticky = 0;
        g_game.ball.stickedBall = 0;
        
        // Determine target speed (consider Slow powerup)
        float targetSpeed = BALL_SPEED;
        if (g_game.slowTimerEndTime > 0) {
            targetSpeed = BALL_SPEED * 0.5f;
        }
        
        // Calculate X speed based on sticky position
        float hitPos = g_game.ball.stickedBallOffsetX / g_game.paddle.width;
        float newSpeedX = (hitPos - 0.5f) * targetSpeed * MAX_BOUNCE_ANGLE;
        newSpeedX = max(-targetSpeed * MAX_BOUNCE_ANGLE, 
                        min(targetSpeed * MAX_BOUNCE_ANGLE, newSpeedX));
        
        // Y speed - inverted, normalized and remembered
        float newSpeedY = -g_game.ball.lastSpeedY;
        
        // Normalize resulting vector to targetSpeed
        float finalSpeed = sqrtf(newSpeedX * newSpeedX + newSpeedY * newSpeedY);
        if (finalSpeed > 0.01f && finalSpeed != targetSpeed) {
            float ratio = targetSpeed / finalSpeed;
            newSpeedX *= ratio;
            newSpeedY *= ratio;
        }
        
        g_game.ball.speedX = newSpeedX;
        g_game.ball.speedY = newSpeedY;
        
        // Move ball away from paddle to avoid immediate collision
        g_game.ball.y = g_game.paddle.y - g_game.ball.radius - 1;
    }
}

static void LoseLife(void) {
    g_game.ball.active = 0;
    g_game.ball.sticky = 0;
    g_game.ball.hasBounced = 0;
    g_game.ball.stickedBall = 0;
    g_game.paddle.lives--;
    PlaySoundEffect("life_lost.wav");
    
    for (int i = 0; i < MAX_POWERUPS; i++) {
        g_game.powerups[i].active = 0;
    }
    ResetPowerupEffects();
    
    if (g_game.paddle.lives > 0) {
        g_game.ball.x = g_game.paddle.x + g_game.paddle.width / 2;
        g_game.ball.y = g_game.paddle.y - g_game.ball.radius;
    } else {
        g_game.gameState = GAME_OVER;
    }
}

/*
=======================
Game Graphics
=======================
*/

static void DrawPlaying(HDC memDC) {
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    RECT rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    FillRect(memDC, &rect, blackBrush);
    DeleteObject(blackBrush);
    
    // Draw bricks
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (g_game.bricks[i][j].active) {
                COLORREF brickColor;
                switch (g_game.bricks[i][j].type) {
                    case BRICK_NORMAL:
                        brickColor = RGB(255, 100, 100);
                        break;
                    case BRICK_HARD:
                        brickColor = (g_game.bricks[i][j].hits == 0) ? RGB(255, 200, 100) : RGB(200, 150, 50);
                        break;
                    case BRICK_STRONG:
                        if (g_game.bricks[i][j].hits == 0)
                            brickColor = RGB(150, 100, 255);
                        else if (g_game.bricks[i][j].hits == 1)
                            brickColor = RGB(100, 50, 200);
                        else
                            brickColor = RGB(50, 0, 150);
                        break;
                    default:
                        brickColor = RGB(255, 255, 255);
                }
                
                HBRUSH brickBrush = CreateSolidBrush(brickColor);
                RECT brickRect = {
                    (int)g_game.bricks[i][j].x,
                    (int)g_game.bricks[i][j].y,
                    (int)(g_game.bricks[i][j].x + BRICK_WIDTH - 1),
                    (int)(g_game.bricks[i][j].y + BRICK_HEIGHT - 1)
                };
                FillRect(memDC, &brickRect, brickBrush);
                DeleteObject(brickBrush);
            }
        }
    }
    
    // Draw paddle
    HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
    RECT paddleRect = {
        (int)g_game.paddle.x,
        (int)g_game.paddle.y,
        (int)(g_game.paddle.x + g_game.paddle.width),
        (int)(g_game.paddle.y + g_game.paddle.height)
    };
    FillRect(memDC, &paddleRect, whiteBrush);
    DeleteObject(whiteBrush);
    
    // Draw powerups
    DrawPowerups(memDC);
    
    // Draw ball
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HBRUSH ballBrush = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(memDC, pen);
    SelectObject(memDC, ballBrush);
    
    Ellipse(memDC, 
            (int)(g_game.ball.x - g_game.ball.radius), 
            (int)(g_game.ball.y - g_game.ball.radius),
            (int)(g_game.ball.x + g_game.ball.radius), 
            (int)(g_game.ball.y + g_game.ball.radius));
    
    DeleteObject(pen);
    DeleteObject(ballBrush);
    
    SetBkMode(memDC, TRANSPARENT);
    
    HFONT textFont = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    HFONT oldFont = SelectObject(memDC, textFont);
    SetTextColor(memDC, RGB(255, 255, 255));
    
    wchar_t leftText[256] = {0};
    wchar_t temp[64];
    
    // Level
    swprintf(temp, 64, L"%ls %d", GetString("LEVEL"), g_game.currentLevel);
    wcscat(leftText, temp);
    
    // Lives
    swprintf(temp, 64, L"  %ls %d", GetString("LIVES"), g_game.paddle.lives);
    wcscat(leftText, temp);
    
    // Score
    swprintf(temp, 64, L"  %ls %d", GetString("SCORE"), g_game.score);
    wcscat(leftText, temp);
    
    // Display in top left corner
    TextOutW(memDC, 10, 10, leftText, wcslen(leftText));

    // Center - Active modes
    int stickyActive = (g_game.stickyTimerEndTime > 0);
    int slowActive = (g_game.slowTimerEndTime > 0);
    
    if (stickyActive || slowActive) {
        wchar_t centerText[128] = {0};
        
        if (stickyActive) {
            wcscat(centerText, GetString("STICKY"));
        }
        if (slowActive) {
            if (stickyActive) wcscat(centerText, L"  ");
            wcscat(centerText, GetString("SLOW"));
        }
        
        SIZE centerSize;
        GetTextExtentPoint32W(memDC, centerText, wcslen(centerText), &centerSize);
        TextOutW(memDC, (SCREEN_WIDTH - centerSize.cx) / 2, 10, centerText, wcslen(centerText));
    }
    
    // Right corner - FPS and ball speed
    // FPS calculation
    static DWORD lastFpsTime = 0;
    static int frameCount = 0;
    static float currentFPS = 0;
    
    frameCount++;
    DWORD currentTime = GetTickCount();
    if (currentTime - lastFpsTime >= 1000) {
        currentFPS = (float)frameCount;
        frameCount = 0;
        lastFpsTime = currentTime;
    }
    
    // Calculate ball speed - show 0 if ball is on paddle
    float ballSpeed;
    if (!g_game.ball.active) {
        ballSpeed = 0.0f;
    } else {
        ballSpeed = sqrtf(g_game.ball.speedX * g_game.ball.speedX + 
                          g_game.ball.speedY * g_game.ball.speedY);
    }
    
    wchar_t rightText[128];
    swprintf(rightText, 128, L"FPS: %.0f  Speed: %.0f", currentFPS, ballSpeed);
    
    SIZE rightSize;
    GetTextExtentPoint32W(memDC, rightText, wcslen(rightText), &rightSize);
    TextOutW(memDC, SCREEN_WIDTH - rightSize.cx - 10, 10, rightText, wcslen(rightText));
    
    // Hint for launching ball
    if (!g_game.ball.active) {
        SetTextColor(memDC, RGB(128, 128, 128));
        const wchar_t* launchText = g_game.paddle.stickyMode ? 
            GetString("PRESS_SPACE_STICKY") : GetString("PRESS_SPACE");
        SIZE launchSize;
        GetTextExtentPoint32W(memDC, launchText, wcslen(launchText), &launchSize);
        TextOutW(memDC, (SCREEN_WIDTH - launchSize.cx) / 2, SCREEN_HEIGHT - 100, 
                launchText, wcslen(launchText));
    }
    
    SelectObject(memDC, oldFont);
    DeleteObject(textFont);
}

void DrawGame(HDC hdc) {
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, SCREEN_WIDTH, SCREEN_HEIGHT);
    HGDIOBJ oldBitmap = SelectObject(memDC, memBitmap);
    
    typedef void (*DrawHandler)(HDC);
    static DrawHandler drawHandlers[] = {
        DrawMenu,
        DrawPlaying,
        DrawGameOver,
        DrawVictory,
        DrawAbout,
        DrawSoundsMenu,
        DrawLanguageMenu
    };
    
    if (g_game.gameState >= 0 && g_game.gameState <= GAME_LANGUAGE) {
        if (g_game.gameState == GAME_PLAYING) {
            DrawPlaying(memDC);
        } else if (drawHandlers[g_game.gameState]) {
            drawHandlers[g_game.gameState](memDC);
        }
    }
    
    BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, memDC, 0, 0, SRCCOPY);
    
    SelectObject(memDC, oldBitmap);
    DeleteDC(memDC);
    DeleteObject(memBitmap);
}

/*
=======================
Game Update
=======================
*/

void UpdateGame() {
    typedef void (*InputHandler)(void);
    static InputHandler inputHandlers[] = {
        HandleMenuInput,
        NULL,
        HandleGameOverInput,
        HandleVictoryInput,
        HandleAboutInput,
        HandleSoundsInput,
        HandleLanguageInput
    };
    
    if (g_game.gameState != GAME_PLAYING) {
        if (g_game.gameState >= 0 && g_game.gameState <= GAME_LANGUAGE && inputHandlers[g_game.gameState]) {
            inputHandlers[g_game.gameState]();
        }
        InvalidateRect(g_game.hwnd, NULL, FALSE);
        return;
    }
    
    DWORD currentTime = GetTickCount();
    float deltaTime = (currentTime - g_game.lastTime) / 1000.0f;
    g_game.lastTime = currentTime;
    
    if (deltaTime > 0.05f) deltaTime = 0.05f;
    
    // Paddle control
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        g_game.paddle.x -= PADDLE_SPEED * deltaTime;
        if (g_game.paddle.x < 0) g_game.paddle.x = 0;
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        g_game.paddle.x += PADDLE_SPEED * deltaTime;
        if (g_game.paddle.x > SCREEN_WIDTH - g_game.paddle.width) 
            g_game.paddle.x = SCREEN_WIDTH - g_game.paddle.width;
    }
    
    // Update powerups
    UpdatePowerups(deltaTime);
    
    // Ball logic
    if (g_game.ball.active) {
        UpdateBall(deltaTime);
        CheckBallCollisions();
        
        if (g_game.totalBricks == 0) {
            NextLevel();
        }
    } else if (g_game.ball.stickedBall) {
        // Ball stuck to paddle - move with it, keep offset
        g_game.ball.x = g_game.paddle.x + g_game.ball.stickedBallOffsetX;
        g_game.ball.y = g_game.paddle.y - g_game.ball.radius;
        
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            LaunchBall();
        }
    } else {
        // Regular inactive ball (after losing life)
        g_game.ball.x = g_game.paddle.x + g_game.paddle.width / 2;
        g_game.ball.y = g_game.paddle.y - g_game.ball.radius;
        
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            LaunchBall();
        }
    }
    
    InvalidateRect(g_game.hwnd, NULL, FALSE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_ERASEBKGND:
            return 1;
            
        case WM_TIMER:
            UpdateGame();
            return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            DrawGame(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}