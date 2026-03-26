#include "arkanoid.h"
#include <stdio.h>
#include <string.h>

extern GameState g_game;
static int selectedItem = 0;
static int soundSelected = 0;
static int langSelected = 0;
static RECT copyButtonRect = {0, 0, 0, 0};

/*
=======================
Helper Functions
=======================
*/

static void ClearBackground(HDC memDC) {
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    RECT rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    FillRect(memDC, &rect, blackBrush);
    DeleteObject(blackBrush);
    SetBkMode(memDC, TRANSPARENT);
}

static HFONT CreateFontBySize(int size, int weight) {
    return CreateFontW(size, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                      DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
}

static void DrawCenteredText(HDC memDC, const wchar_t* text, int y, HFONT font, COLORREF color) {
    HFONT oldFont = SelectObject(memDC, font);
    SetTextColor(memDC, color);
    
    SIZE size;
    GetTextExtentPoint32W(memDC, text, wcslen(text), &size);
    TextOutW(memDC, (SCREEN_WIDTH - size.cx) / 2, y, text, wcslen(text));
    
    SelectObject(memDC, oldFont);
}

static void DrawLeftText(HDC memDC, const wchar_t* text, int x, int y, HFONT font, COLORREF color) {
    HFONT oldFont = SelectObject(memDC, font);
    SetTextColor(memDC, color);
    TextOutW(memDC, x, y, text, wcslen(text));
    SelectObject(memDC, oldFont);
}

static void DrawRightText(HDC memDC, const wchar_t* text, int x, int y, HFONT font, COLORREF color) {
    HFONT oldFont = SelectObject(memDC, font);
    SetTextColor(memDC, color);
    
    SIZE size;
    GetTextExtentPoint32W(memDC, text, wcslen(text), &size);
    TextOutW(memDC, x - size.cx, y, text, wcslen(text));
    
    SelectObject(memDC, oldFont);
}

static void HandleNavigation(int* selected, int min, int max) {
    static DWORD lastKeyTime = 0;
    DWORD currentTime = GetTickCount();
    
    if (currentTime - lastKeyTime < 150) return;
    
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        (*selected)--;
        if (*selected < min) *selected = max;
        lastKeyTime = currentTime;
    }
    else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        (*selected)++;
        if (*selected > max) *selected = min;
        lastKeyTime = currentTime;
    }
}

/*
======================
Menu Drawing Functions
======================
*/

void DrawMenu(HDC memDC) {
    ClearBackground(memDC);
    
    HFONT bigFont = CreateFontBySize(72, FW_BOLD);
    HFONT menuFont = CreateFontBySize(32, FW_NORMAL);
    HFONT smallFont = CreateFontBySize(16, FW_NORMAL);
    
    DrawCenteredText(memDC, L"ARKANOID", 100, bigFont, RGB(255, 215, 0));
    
    const char* menuKeys[] = {"START_GAME", "ABOUT", "SOUNDS", "LANGUAGE", "EXIT"};
    for (int i = 0; i < 5; i++) {
        COLORREF color = (i == selectedItem) ? RGB(255, 255, 0) : RGB(255, 255, 255);
        DrawCenteredText(memDC, GetString(menuKeys[i]), 250 + i * 55, menuFont, color);
    }
    
    DrawCenteredText(memDC, GetString("MENU_HINT"), SCREEN_HEIGHT - 50, smallFont, RGB(128, 128, 128));
    
    DeleteObject(bigFont);
    DeleteObject(menuFont);
    DeleteObject(smallFont);
}

void DrawSoundsMenu(HDC memDC) {
    ClearBackground(memDC);
    
    HFONT titleFont = CreateFontBySize(48, FW_BOLD);
    HFONT textFont = CreateFontBySize(32, FW_NORMAL);
    HFONT smallFont = CreateFontBySize(16, FW_NORMAL);
    
    DrawCenteredText(memDC, GetString("SOUND_SETTINGS"), 80, titleFont, RGB(255, 215, 0));
    
    int startY = 200;
    int spacing = 60;
    
    COLORREF sfxColor = (soundSelected == 0) ? RGB(255, 255, 0) : RGB(255, 255, 255);
    DrawLeftText(memDC, GetString("SFX"), SCREEN_WIDTH / 2 - 150 + 45, startY, textFont, sfxColor);
    
    COLORREF statusColor = (soundSelected == 0) ? RGB(255, 255, 0) : RGB(255, 255, 255);
    DrawRightText(memDC, g_game.sfxEnabled ? GetString("ON") : GetString("OFF"), 
                  SCREEN_WIDTH / 2 + 50 + 45, startY, textFont, statusColor);
    
    COLORREF backColor = (soundSelected == 1) ? RGB(255, 255, 0) : RGB(255, 255, 255);
    DrawCenteredText(memDC, GetString("BACK_TO_MENU"), startY + spacing, textFont, backColor);
    
    const wchar_t* hint = (soundSelected == 0) ? 
        GetString("SOUNDS_HINT_ADJUST") : GetString("SOUNDS_HINT_BACK");
    DrawCenteredText(memDC, hint, SCREEN_HEIGHT - 50, smallFont, RGB(128, 128, 128));
    
    DeleteObject(titleFont);
    DeleteObject(textFont);
    DeleteObject(smallFont);
}

void DrawLanguageMenu(HDC memDC) {
    ClearBackground(memDC);
    
    HFONT titleFont = CreateFontBySize(48, FW_BOLD);
    HFONT textFont = CreateFontBySize(32, FW_NORMAL);
    HFONT smallFont = CreateFontBySize(16, FW_NORMAL);
    
    DrawCenteredText(memDC, GetString("LANGUAGE_SETTINGS"), 80, titleFont, RGB(255, 215, 0));
    
    int startY = 200;
    int spacing = 60;
    
    COLORREF enColor = (langSelected == 0) ? RGB(255, 255, 0) : RGB(255, 255, 255);
    DrawCenteredText(memDC, GetString("ENGLISH"), startY, textFont, enColor);
    
    COLORREF ruColor = (langSelected == 1) ? RGB(255, 255, 0) : RGB(255, 255, 255);
    DrawCenteredText(memDC, GetString("RUSSIAN"), startY + spacing, textFont, ruColor);
    
    COLORREF backColor = (langSelected == 2) ? RGB(255, 255, 0) : RGB(255, 255, 255);
    DrawCenteredText(memDC, GetString("BACK_TO_MENU"), startY + spacing * 2, textFont, backColor);
    
    DrawCenteredText(memDC, GetString("LANGUAGE_HINT"), SCREEN_HEIGHT - 50, smallFont, RGB(128, 128, 128));
    
    DeleteObject(titleFont);
    DeleteObject(textFont);
    DeleteObject(smallFont);
}

void DrawAbout(HDC memDC) {
    ClearBackground(memDC);
    
    HFONT titleFont = CreateFontBySize(48, FW_BOLD);
    HFONT textFont = CreateFontBySize(24, FW_NORMAL);
    
    DrawCenteredText(memDC, GetString("ABOUT_TITLE"), 35, titleFont, RGB(255, 215, 0));
    DrawCenteredText(memDC, GetString("VERSION"), 85, textFont, RGB(255, 255, 255));
    
    int y = 135;
    int lineHeight = 32;
    int leftColumnX = 70;
    int rightColumnX = 480;
    
    DrawLeftText(memDC, GetString("CONTROLS"), leftColumnX, y, textFont, RGB(255, 255, 255));
    y += lineHeight;
    DrawLeftText(memDC, GetString("MOVE_PADDLE"), leftColumnX + 25, y, textFont, RGB(255, 255, 255));
    y += lineHeight;
    DrawLeftText(memDC, GetString("LAUNCH_BALL"), leftColumnX + 25, y, textFont, RGB(255, 255, 255));
    
    y = 135;
    DrawLeftText(memDC, GetString("POWERUPS_TITLE"), rightColumnX, y, textFont, RGB(255, 255, 255));
    y += lineHeight;
    DrawLeftText(memDC, GetString("POWERUP_ENLARGE"), rightColumnX + 25, y, textFont, RGB(255, 255, 255));
    y += lineHeight;
    DrawLeftText(memDC, GetString("POWERUP_SHRINK"), rightColumnX + 25, y, textFont, RGB(255, 255, 255));
    y += lineHeight;
    DrawLeftText(memDC, GetString("POWERUP_SLOW"), rightColumnX + 25, y, textFont, RGB(255, 255, 255));
    y += lineHeight;
    DrawLeftText(memDC, GetString("POWERUP_STICKY"), rightColumnX + 25, y, textFont, RGB(255, 255, 255));
    y += lineHeight;
    DrawLeftText(memDC, GetString("POWERUP_EXTRA_LIFE"), rightColumnX + 25, y, textFont, RGB(255, 255, 255));
    
    y = y + 15;
    SetTextColor(memDC, RGB(100, 200, 100));
    
    const wchar_t* donateTitle = (g_game.currentLanguage == LANG_RU) ? L"Поддержать:" : L"Donate:";
    const wchar_t* donateAddr1 = L"TRC20 USDT/TRX:";
    const wchar_t* donateAddr2 = L"TTjnMhCcus7cibpAyx7PqaiQPuu4L6NV1a";
    const wchar_t* copyright = L"(c) 2026 playtester";
    
    DrawCenteredText(memDC, donateTitle, y, textFont, RGB(100, 200, 100));
    y += 28;
    DrawCenteredText(memDC, donateAddr1, y, textFont, RGB(100, 200, 100));
    y += 30;
    DrawCenteredText(memDC, donateAddr2, y, textFont, RGB(255, 200, 100));
    y += 38;
    
    int copyBtnX = (SCREEN_WIDTH - 80) / 2;
    int copyBtnY = y;
    int copyBtnW = 80;
    int copyBtnH = 30;
    
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    HPEN oldPen = SelectObject(memDC, pen);
    HBRUSH brush = CreateSolidBrush(RGB(60, 60, 60));
    HBRUSH oldBrush = SelectObject(memDC, brush);
    Rectangle(memDC, copyBtnX, copyBtnY, copyBtnX + copyBtnW, copyBtnY + copyBtnH);
    SelectObject(memDC, oldPen);
    SelectObject(memDC, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
    
    DrawCenteredText(memDC, L"COPY", copyBtnY + (copyBtnH - 24) / 2, textFont, RGB(255, 255, 255));
    
    copyButtonRect.left = copyBtnX;
    copyButtonRect.top = copyBtnY;
    copyButtonRect.right = copyBtnX + copyBtnW;
    copyButtonRect.bottom = copyBtnY + copyBtnH;
    
    y += 45;
    DrawCenteredText(memDC, copyright, y, textFont, RGB(100, 100, 100));
    
    int backY = y + 45;
    DrawCenteredText(memDC, GetString("BACK_TO_MENU"), backY, textFont, RGB(255, 255, 0));
    
    DeleteObject(titleFont);
    DeleteObject(textFont);
}

void DrawGameOver(HDC memDC) {
    ClearBackground(memDC);
    
    HFONT bigFont = CreateFontBySize(72, FW_BOLD);
    HFONT textFont = CreateFontBySize(32, FW_NORMAL);
    
    DrawCenteredText(memDC, GetString("GAME_OVER"), SCREEN_HEIGHT / 2 - 80, bigFont, RGB(255, 0, 0));
    
    wchar_t text[100];
    swprintf(text, 100, L"%ls %d", GetString("FINAL_SCORE"), g_game.score);
    DrawCenteredText(memDC, text, SCREEN_HEIGHT / 2, textFont, RGB(255, 255, 255));
    DrawCenteredText(memDC, GetString("BACK_TO_MENU"), SCREEN_HEIGHT - 100, textFont, RGB(255, 255, 0));
    
    DeleteObject(bigFont);
    DeleteObject(textFont);
}

void DrawVictory(HDC memDC) {
    ClearBackground(memDC);
    
    HFONT bigFont = CreateFontBySize(72, FW_BOLD);
    HFONT textFont = CreateFontBySize(32, FW_NORMAL);
    
    DrawCenteredText(memDC, GetString("VICTORY"), SCREEN_HEIGHT / 2 - 80, bigFont, RGB(0, 255, 0));
    
    wchar_t text[100];
    swprintf(text, 100, L"%ls %d", GetString("FINAL_SCORE"), g_game.score);
    DrawCenteredText(memDC, text, SCREEN_HEIGHT / 2, textFont, RGB(255, 255, 255));
    DrawCenteredText(memDC, GetString("BACK_TO_MENU"), SCREEN_HEIGHT - 100, textFont, RGB(255, 255, 0));
    
    DeleteObject(bigFont);
    DeleteObject(textFont);
}

/*
=======================
Input Handling
=======================
*/

void HandleMenuInput() {
    HandleNavigation(&selectedItem, 0, 4);
    
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
        switch (selectedItem) {
            case 0:
                g_game.gameState = GAME_PLAYING;
                ResetGame();
                break;
            case 1:
                g_game.gameState = GAME_ABOUT;
                break;
            case 2:
                g_game.gameState = GAME_SOUNDS;
                soundSelected = 0;
                break;
            case 3:
                g_game.gameState = GAME_LANGUAGE;
                langSelected = (g_game.currentLanguage == LANG_EN) ? 0 : 1;
                break;
            case 4:
                PostQuitMessage(0);
                break;
        }
        Sleep(200);
    }
}

void HandleSoundsInput() {
    static DWORD lastToggleTime = 0;
    DWORD currentTime = GetTickCount();
    
    if (soundSelected == 0) {
        if ((GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000) &&
            currentTime - lastToggleTime > 150) {
            SetSFXEnabled(!g_game.sfxEnabled);
            lastToggleTime = currentTime;
        }
        HandleNavigation(&soundSelected, 0, 1);
    } else {
        HandleNavigation(&soundSelected, 0, 1);
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
            g_game.gameState = GAME_MENU;
            Sleep(200);
        }
    }
}

void HandleLanguageInput() {
    HandleNavigation(&langSelected, 0, 2);
    
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
        if (langSelected == 0) {
            SetLanguage(LANG_EN);
            g_game.gameState = GAME_MENU;
        } else if (langSelected == 1) {
            SetLanguage(LANG_RU);
            g_game.gameState = GAME_MENU;
        } else if (langSelected == 2) {
            g_game.gameState = GAME_MENU;
        }
        Sleep(200);
    }
}

void HandleAboutInput() {
    POINT cursor;
    GetCursorPos(&cursor);
    ScreenToClient(g_game.hwnd, &cursor);
    
    static DWORD lastClickTime = 0;
    DWORD currentTime = GetTickCount();
    
    if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && currentTime - lastClickTime > 200) {
        if (cursor.x >= copyButtonRect.left && cursor.x <= copyButtonRect.right &&
            cursor.y >= copyButtonRect.top && cursor.y <= copyButtonRect.bottom) {
            
            const wchar_t* address = L"TTjnMhCcus7cibpAyx7PqaiQPuu4L6NV1a";
            if (OpenClipboard(g_game.hwnd)) {
                EmptyClipboard();
                int size = (wcslen(address) + 1) * sizeof(wchar_t);
                HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
                if (hGlobal) {
                    memcpy(GlobalLock(hGlobal), address, size);
                    GlobalUnlock(hGlobal);
                    SetClipboardData(CF_UNICODETEXT, hGlobal);
                }
                CloseClipboard();
                MessageBeep(MB_OK);
            }
            lastClickTime = currentTime;
            return;
        }
        
        int backY = SCREEN_HEIGHT - 45;
        const wchar_t* backText = GetString("BACK_TO_MENU");
        SIZE backSize;
        HDC hdc = GetDC(g_game.hwnd);
        HFONT font = CreateFontBySize(24, FW_NORMAL);
        HFONT oldFont = SelectObject(hdc, font);
        GetTextExtentPoint32W(hdc, backText, wcslen(backText), &backSize);
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        ReleaseDC(g_game.hwnd, hdc);
        
        int backX = (SCREEN_WIDTH - backSize.cx) / 2;
        if (cursor.x >= backX && cursor.x <= backX + backSize.cx &&
            cursor.y >= backY && cursor.y <= backY + 30) {
            g_game.gameState = GAME_MENU;
            lastClickTime = currentTime;
        }
    }
    
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
        g_game.gameState = GAME_MENU;
        Sleep(200);
    }
}

void HandleGameOverInput() {
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
        g_game.gameState = GAME_MENU;
        Sleep(200);
    }
}

void HandleVictoryInput() {
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
        g_game.gameState = GAME_MENU;
        Sleep(200);
    }
}