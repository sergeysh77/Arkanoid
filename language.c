#include "arkanoid.h"
#include <stdio.h>
#include <string.h>

extern GameState g_game;

typedef struct {
    const char* key;
    const wchar_t* en;
    const wchar_t* ru;
} LanguageString;

static LanguageString strings[] = {
    {"START_GAME", L"START GAME", L"НАЧАТЬ ИГРУ"},
    {"ABOUT", L"ABOUT", L"О ИГРЕ"},
    {"SOUNDS", L"SOUNDS", L"ЗВУК"},
    {"LANGUAGE", L"LANGUAGE", L"ЯЗЫК"},
    {"EXIT", L"EXIT", L"ВЫХОД"},
    {"BACK_TO_MENU", L"BACK TO MENU", L"НАЗАД В МЕНЮ"},
    {"MENU_HINT", L"Use UP/DOWN arrows to select, ENTER to confirm", L"Используйте СТРЕЛКИ ВВЕРХ/ВНИЗ для выбора, ENTER для подтверждения"},
    {"SOUNDS_HINT_ADJUST", L"LEFT/RIGHT: toggle ON/OFF   ENTER on BACK: return to menu", L"ВЛЕВО/ВПРАВО: включить/выключить   ENTER на BACK: вернуться в меню"},
    {"SOUNDS_HINT_BACK", L"UP/DOWN: select   ENTER: return to menu", L"ВВЕРХ/ВНИЗ: выбор   ENTER: вернуться в меню"},
    {"LANGUAGE_HINT", L"UP/DOWN: select   ENTER: confirm and return to menu", L"ВВЕРХ/ВНИЗ: выбор   ENTER: подтвердить и вернуться в меню"},
    {"SCORE", L"Score:", L"Счет:"},
    {"LIVES", L"Lives:", L"Жизни:"},
    {"LEVEL", L"Level:", L"Уровень:"},
    {"STICKY", L"STICKY", L"ЛИПКИЙ"},
    {"SLOW", L"SLOW", L"МЕДЛЕННО"},
    {"PRESS_SPACE", L"Press SPACE to launch the ball", L"Нажмите ПРОБЕЛ для запуска мяча"},
    {"PRESS_SPACE_STICKY", L"Press SPACE to launch the ball (STICKY MODE)", L"Нажмите ПРОБЕЛ для запуска мяча (ЛИПКИЙ РЕЖИМ)"},
    {"VICTORY", L"VICTORY!", L"ПОБЕДА!"},
    {"GAME_OVER", L"GAME OVER", L"ИГРА ОКОНЧЕНА"},
    {"FINAL_SCORE", L"Final Score:", L"Финальный счет:"},
    {"SOUND_SETTINGS", L"SOUND SETTINGS", L"НАСТРОЙКИ ЗВУКА"},
    {"SFX", L"Sounds:", L"Звуки:"},
    {"ON", L"ON", L"ВКЛ"},
    {"OFF", L"OFF", L"ВЫКЛ"},
    {"LANGUAGE_SETTINGS", L"LANGUAGE SETTINGS", L"НАСТРОЙКИ ЯЗЫКА"},
    {"ENGLISH", L"ENGLISH", L"АНГЛИЙСКИЙ"},
    {"RUSSIAN", L"RUSSIAN", L"РУССКИЙ"},
    {"ABOUT_TITLE", L"ABOUT", L"О ИГРЕ"},
    {"VERSION", L"Version 1.0", L"Версия 1.0"},
    {"CONTROLS", L"Controls:", L"Управление:"},
    {"MOVE_PADDLE", L"LEFT/RIGHT arrows - move paddle", L"ВЛЕВО/ВПРАВО - движения"},
    {"LAUNCH_BALL", L"SPACE - launch ball", L"ПРОБЕЛ - запуск мяча"},
    {"POWERUPS_TITLE", L"Powerups:", L"Бонусы:"},
    {"POWERUP_ENLARGE", L"L - enlarge paddle", L"L - увеличение платформы"},
    {"POWERUP_SHRINK", L"S - shrink paddle", L"S - уменьшение платформы"},
    {"POWERUP_SLOW", L"W - slow ball", L"W - замедление мяча"},
    {"POWERUP_STICKY", L"T - sticky ball", L"T - липкий мяч"},
    {"POWERUP_EXTRA_LIFE", L"+ - extra life", L"+ - дополнительная жизнь"},
    {NULL, NULL, NULL}
};

void InitLanguage() {
    LANGID langId = GetSystemDefaultUILanguage();
    if (langId == 1049 || langId == 2073) {
        g_game.currentLanguage = LANG_RU;
    } else {
        g_game.currentLanguage = LANG_EN;
    }
}

void SetLanguage(int lang) {
    g_game.currentLanguage = lang;
}

const wchar_t* GetString(const char* key) {
    for (int i = 0; strings[i].key != NULL; i++) {
        if (strcmp(strings[i].key, key) == 0) {
            if (g_game.currentLanguage == LANG_EN) {
                return strings[i].en ? strings[i].en : L"";
            } else {
                return strings[i].ru ? strings[i].ru : L"";
            }
        }
    }
    return L"";
}