#include "arkanoid.h"

// Brick data structure
typedef struct {
    int type;
    int hits;
    int maxHits;
} BrickData;

// Level data
static BrickData level1[BRICK_ROWS][BRICK_COLS] = {
    {{BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1},
     {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}},
    
    {{BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1},
     {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}},
    
    {{BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1},
     {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2}},
    
    {{BRICK_HARD, 0, 2}, {BRICK_STRONG, 0, 3}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_EMPTY, 0, 0},
     {BRICK_EMPTY, 0, 0}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_STRONG, 0, 3}, {BRICK_HARD, 0, 2}},
    
    {{BRICK_STRONG, 0, 3}, {BRICK_STRONG, 0, 3}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1},
     {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_STRONG, 0, 3}, {BRICK_STRONG, 0, 3}}
};

static BrickData level2[BRICK_ROWS][BRICK_COLS] = {
    {{BRICK_EMPTY, 0, 0}, {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2},
     {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2}, {BRICK_EMPTY, 0, 0}},
    
    {{BRICK_EMPTY, 0, 0}, {BRICK_HARD, 0, 2}, {BRICK_STRONG, 0, 3}, {BRICK_STRONG, 0, 3}, {BRICK_STRONG, 0, 3},
     {BRICK_STRONG, 0, 3}, {BRICK_STRONG, 0, 3}, {BRICK_STRONG, 0, 3}, {BRICK_HARD, 0, 2}, {BRICK_EMPTY, 0, 0}},
    
    {{BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2},
     {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2}, {BRICK_HARD, 0, 2}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}},
    
    {{BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_STRONG, 0, 3}, {BRICK_STRONG, 0, 3},
     {BRICK_STRONG, 0, 3}, {BRICK_STRONG, 0, 3}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}},
    
    {{BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_STRONG, 0, 3},
     {BRICK_STRONG, 0, 3}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}, {BRICK_NORMAL, 0, 1}}
};

// Array of level pointers
static BrickData (*levels[])[BRICK_ROWS][BRICK_COLS] = {
    NULL,           // index 0 not used
    &level1,
    &level2,
    // add new levels here
};

void LoadLevel(int levelNum, void* bricksPtr, int brickRows, int brickCols, int brickWidth, int brickHeight, int startY, int* totalBricks) {
    Brick* bricks = (Brick*)bricksPtr;
    *totalBricks = 0;
    
    if (levelNum < 1 || levelNum >= (int)(sizeof(levels)/sizeof(levels[0]))) {
        levelNum = 1;
    }
    
    BrickData (*currentLevel)[BRICK_ROWS][BRICK_COLS] = levels[levelNum];
    
    for (int i = 0; i < brickRows; i++) {
        for (int j = 0; j < brickCols; j++) {
            BrickData data = (*currentLevel)[i][j];
            int idx = i * brickCols + j;
            
            if (data.type != BRICK_EMPTY) {
                bricks[idx].active = 1;
                bricks[idx].hits = data.hits;
                bricks[idx].maxHits = data.maxHits;
                bricks[idx].type = data.type;
                bricks[idx].x = j * brickWidth;
                bricks[idx].y = i * brickHeight + startY;
                (*totalBricks)++;
            } else {
                bricks[idx].active = 0;
                bricks[idx].type = BRICK_EMPTY;
            }
        }
    }
}