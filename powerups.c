#include "arkanoid.h"
#include <stdlib.h>
#include <math.h>

extern GameState g_game;

void SpawnPowerup(float x, float y) {
    if (rand() % 100 < 10) {
        for (int i = 0; i < MAX_POWERUPS; i++) {
            if (!g_game.powerups[i].active) {
                g_game.powerups[i].x = x + BRICK_WIDTH / 2;
                g_game.powerups[i].y = y;
                g_game.powerups[i].active = 1;
                g_game.powerups[i].speedY = 200.0f;
                g_game.powerups[i].type = (rand() % 5) + 1;
                break;
            }
        }
    }
}

void UpdatePowerups(float deltaTime) {
    DWORD currentTime = GetTickCount();
    
    // Update falling powerups
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (g_game.powerups[i].active) {
            g_game.powerups[i].y += g_game.powerups[i].speedY * deltaTime;
            
            if (g_game.powerups[i].y + 10 >= g_game.paddle.y && 
                g_game.powerups[i].y - 10 <= g_game.paddle.y + g_game.paddle.height &&
                g_game.powerups[i].x + 10 >= g_game.paddle.x && 
                g_game.powerups[i].x - 10 <= g_game.paddle.x + g_game.paddle.width) {
                
                ApplyPowerup(g_game.powerups[i].type);
                PlaySoundEffect("powerup.wav");
                g_game.powerups[i].active = 0;
            }
            
            if (g_game.powerups[i].y > SCREEN_HEIGHT) {
                g_game.powerups[i].active = 0;
            }
        }
    }
    
    // Check sticky powerup
    if (g_game.stickyTimerEndTime > 0 && currentTime >= g_game.stickyTimerEndTime) {
        g_game.paddle.stickyMode = 0;
        g_game.stickyTimerEndTime = 0;
    }
    
    // Check slow powerup
    if (g_game.slowTimerEndTime > 0 && currentTime >= g_game.slowTimerEndTime) {
        // Restore normal speed 300
        float currentSpeed = sqrtf(g_game.ball.speedX * g_game.ball.speedX + 
                                    g_game.ball.speedY * g_game.ball.speedY);
        // Always restore to BALL_SPEED (300)
        if (currentSpeed != BALL_SPEED && currentSpeed > 0) {
            float ratio = BALL_SPEED / currentSpeed;
            g_game.ball.speedX *= ratio;
            g_game.ball.speedY *= ratio;
        }
        g_game.slowTimerEndTime = 0;
    }
}

void DrawPowerups(HDC memDC) {
    SetBkMode(memDC, TRANSPARENT);
    
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (g_game.powerups[i].active) {
            COLORREF color;
            char symbol;
            
            switch (g_game.powerups[i].type) {
                case POWERUP_ENLARGE:
                    color = RGB(0, 255, 0);
                    symbol = 'L';
                    break;
                case POWERUP_SHRINK:
                    color = RGB(255, 0, 0);
                    symbol = 'S';
                    break;
                case POWERUP_SLOW:
                    color = RGB(0, 255, 255);
                    symbol = 'W';
                    break;
                case POWERUP_STICKY:
                    color = RGB(255, 255, 0);
                    symbol = 'T';
                    break;
                case POWERUP_EXTRA_LIFE:
                    color = RGB(255, 0, 255);
                    symbol = '+';
                    break;
                default:
                    color = RGB(255, 255, 255);
                    symbol = '?';
            }
            
            HBRUSH brush = CreateSolidBrush(color);
            RECT rect = {
                (int)(g_game.powerups[i].x - 8),
                (int)(g_game.powerups[i].y - 8),
                (int)(g_game.powerups[i].x + 8),
                (int)(g_game.powerups[i].y + 8)
            };
            FillRect(memDC, &rect, brush);
            DeleteObject(brush);
            
            SetTextColor(memDC, RGB(0, 0, 0));
            char text[2] = {symbol, '\0'};
            TextOutA(memDC, (int)(g_game.powerups[i].x - 4), (int)(g_game.powerups[i].y - 6), text, 1);
        }
    }
}

void ApplyPowerup(int type) {
    DWORD currentTime = GetTickCount();
    DWORD durationMs = 45000;  // 45 seconds
    
    switch (type) {
        case POWERUP_ENLARGE:
            if (g_game.paddle.width < g_game.paddle.originalWidth * 1.5f) {
                g_game.paddle.width = min(g_game.paddle.width + 30, g_game.paddle.originalWidth * 1.5f);
                if (g_game.paddle.x + g_game.paddle.width > SCREEN_WIDTH) {
                    g_game.paddle.x = SCREEN_WIDTH - g_game.paddle.width;
                }
            }
            break;
            
        case POWERUP_SHRINK:
            if (g_game.paddle.width > g_game.paddle.originalWidth * 0.5f) {
                g_game.paddle.width = max(g_game.paddle.width - 30, g_game.paddle.originalWidth * 0.5f);
                if (g_game.paddle.x + g_game.paddle.width > SCREEN_WIDTH) {
                    g_game.paddle.x = SCREEN_WIDTH - g_game.paddle.width;
                }
            }
            break;
            
        case POWERUP_SLOW:
            // Record end time
            g_game.slowTimerEndTime = currentTime + durationMs;
            
            // Slow down ball to 150
            float currentSpeed = sqrtf(g_game.ball.speedX * g_game.ball.speedX + 
                                        g_game.ball.speedY * g_game.ball.speedY);
            float targetSpeed = BALL_SPEED * 0.5f;  // 150
            if (currentSpeed > targetSpeed) {
                float ratio = targetSpeed / currentSpeed;
                g_game.ball.speedX *= ratio;
                g_game.ball.speedY *= ratio;
            }
            break;
            
        case POWERUP_STICKY:
            g_game.stickyTimerEndTime = currentTime + durationMs;
            g_game.paddle.stickyMode = 1;
            break;
            
        case POWERUP_EXTRA_LIFE:
            g_game.paddle.lives++;
            break;
    }
}

void ResetPowerupEffects() {
    // Reset powerup effects
    g_game.paddle.width = g_game.paddle.originalWidth;
    g_game.paddle.stickyMode = 0;
    g_game.stickyTimerEndTime = 0;
    g_game.slowTimerEndTime = 0;
    
    // Restore normal ball speed
    float currentSpeed = sqrtf(g_game.ball.speedX * g_game.ball.speedX + 
                                g_game.ball.speedY * g_game.ball.speedY);
    if (currentSpeed != BALL_SPEED && currentSpeed > 0) {
        float ratio = BALL_SPEED / currentSpeed;
        g_game.ball.speedX *= ratio;
        g_game.ball.speedY *= ratio;
    }
}