#ifndef ARKANOID_H
#define ARKANOID_H

#include <windows.h>
#include <mmsystem.h>

// Constants
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 15
#define BALL_SIZE 8
#define BRICK_ROWS 5
#define BRICK_COLS 10
#define BRICK_WIDTH (SCREEN_WIDTH / BRICK_COLS)
#define BRICK_HEIGHT 25
#define TIMER_ID 1
#define TIMER_SPEED 8
#define BALL_SPEED 300.0f
#define MAX_POWERUPS 10
#define TOTAL_LEVELS 2
#define PADDLE_SPEED 800.0f
#define POWERUP_DURATION_SECONDS 45.0f
#define MAX_BOUNCE_ANGLE 1.2f
#define MIN_BOUNCE_ANGLE 0.7f
#define MAX_SPEED_FACTOR 1.3f
#define MIN_SPEED_FACTOR 0.7f

// Brick types
#define BRICK_NORMAL 1
#define BRICK_HARD 2
#define BRICK_STRONG 3
#define BRICK_EMPTY 0

// Powerup types
#define POWERUP_NONE 0
#define POWERUP_ENLARGE 1
#define POWERUP_SHRINK 2
#define POWERUP_SLOW 3
#define POWERUP_STICKY 4
#define POWERUP_EXTRA_LIFE 5

// Game states
#define GAME_MENU 0
#define GAME_PLAYING 1
#define GAME_OVER 2
#define GAME_VICTORY 3
#define GAME_ABOUT 4
#define GAME_SOUNDS 5
#define GAME_LANGUAGE 6

// Languages
#define LANG_EN 0
#define LANG_RU 1

// Structures
typedef struct {
    float x, y;
    float speedX, speedY;
    int radius;
    int active;
    int sticky;
    int hasBounced;
    float lastSpeedX;
    float lastSpeedY;
    int stickedBall;
    float stickedBallOffsetX;
} Ball;

typedef struct {
    float x, y;
    float width, height;
    int lives;
    float originalWidth;
    int stickyMode;
} Paddle;

typedef struct {
    float x, y;
    int active;
    int hits;
    int maxHits;
    int type;
} Brick;

typedef struct {
    float x, y;
    int active;
    int type;
    float speedY;
} Powerup;

// Game state structure
typedef struct {
    Paddle paddle;
    Ball ball;
    Brick bricks[BRICK_ROWS][BRICK_COLS];
    Powerup powerups[MAX_POWERUPS];
    int totalBricks;
    int score;
    int gameState;
    int currentLevel;
    DWORD stickyTimerEndTime;
    DWORD slowTimerEndTime; 
    DWORD lastTime;
    HWND hwnd;
    int sfxEnabled;
    int currentLanguage;
} GameState;

// Global variable
extern GameState g_game;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DrawGame(HDC hdc);
void UpdateGame();
void ResetGame();
void LoadLevelData();
void NextLevel();

// Prototypes from levels.c
void LoadLevel(int levelNum, void* bricksPtr, int brickRows, int brickCols, int brickWidth, int brickHeight, int startY, int* totalBricks);

// Prototypes from powerups.c
void SpawnPowerup(float x, float y);
void UpdatePowerups(float deltaTime);
void DrawPowerups(HDC memDC);
void ApplyPowerup(int type);
void ResetPowerupEffects();

// Prototypes from menu.c
void DrawMenu(HDC memDC);
void HandleMenuInput();
void DrawAbout(HDC memDC);
void HandleAboutInput();
void HandleGameOverInput();
void DrawGameOver(HDC memDC);
void DrawVictory(HDC memDC);
void HandleVictoryInput();
void DrawSoundsMenu(HDC memDC);
void HandleSoundsInput();
void DrawLanguageMenu(HDC memDC);
void HandleLanguageInput();

// Prototypes from sounds.c
void InitSound();
void PlaySoundEffect(const char* soundFile);
void SetSFXEnabled(int enabled);
void CleanupSound();

// Prototypes from language.c
void InitLanguage();
const wchar_t* GetString(const char* key);
void SetLanguage(int lang);

#endif