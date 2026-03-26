#include "arkanoid.h"
#include "resources.h"
#include <stdio.h>
#include <string.h>
#include <mmsystem.h>

extern GameState g_game;

void InitSound() {
    waveOutGetNumDevs();
    g_game.sfxEnabled = 1;
}

void SetSFXEnabled(int enabled) {
    g_game.sfxEnabled = enabled;
}

void PlaySoundEffect(const char* soundFile) {
    if (!g_game.sfxEnabled) return;
    
    int resourceId = -1;
    
    if (strcmp(soundFile, "paddle_hit.wav") == 0) {
        resourceId = SOUND_PADDLE_HIT;
    } else if (strcmp(soundFile, "hit_brick.wav") == 0) {
        resourceId = SOUND_HIT_BRICK;
    } else if (strcmp(soundFile, "life_lost.wav") == 0) {
        resourceId = SOUND_LIFE_LOST;
    } else if (strcmp(soundFile, "powerup.wav") == 0) {
        resourceId = SOUND_POWERUP;
    } else {
        return;
    }
    
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourceId), "WAVE");
    if (hRes) {
        HGLOBAL hData = LoadResource(NULL, hRes);
        if (hData) {
            void* pData = LockResource(hData);
            PlaySound(pData, NULL, SND_MEMORY | SND_ASYNC);
            UnlockResource(hData);
            FreeResource(hData);
        }
    }
}

void CleanupSound() {
    PlaySound(NULL, NULL, 0);
}