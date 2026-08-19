/*
 * Copyright (C) 2023 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

/**
 * @file  patch.c
 * @brief Patching some of the .so internal functions or bridging them to native
 *        for better compatibility.
 */

#include <kubridge.h>
#include <so_util/so_util.h>
#include <sys/stat.h>
#include <stdio.h>
#include "utils/logger.h"

extern so_module so_mod;
extern gameState;

static so_hook Grp_drawWPNImage_hook;
static so_hook Grp_drawWPNImage2_hook;
static so_hook drawAGDImageT_hook;

static so_hook LogoCanvas_init_hook;
static so_hook MenuCanvas_init_hook;
static so_hook GameCanvas_init_hook;
static so_hook GameUI_searchBag_hook;

int hudItems[] = {
        -1744355708, 
        -1744355828, 
        -1744355792, 
        -1744355804,
    };

int hudHightlights[] = {
        -1744355832,
        -1744355816
    };

const int hudItemsCount = sizeof(hudItems) / sizeof(hudItems[0]);
const int hudHightlightsCount = sizeof(hudHightlights) / sizeof(hudHightlights[0]);

void Grp_drawWPNImage_patched(void *this, int param1, int param2, int param3, unsigned int param4) {   
    for (int i = 0; i < hudItemsCount; i++) {
        if (hudItems[i] == param3) {
            return;
        }
    }
    SO_CONTINUE(void *, Grp_drawWPNImage_hook, this, param1, param2, param3, param4);
}

void Grp_drawWPNImage2_patched(void *this, int param1, int param2, int param3, unsigned int param4, signed int param5) {
    return;
}

int drawAGDImageT_patched(void *this, int a2, int a3, int a4, unsigned int* a5, int a6, int a7, int a8, unsigned int a9) {
    for (int i = 0; i < hudHightlightsCount; i++) {
        if (hudHightlights[i] == a4) {
            return;
        }
    }
    return SO_CONTINUE(void *, drawAGDImageT_hook, this, a2, a3, a4, a5, a6, a7, a8, a9);
}

void LogoCanvas_init_patched(){
    l_debug("[LogoCanvas_init] called");
    gameState = 0;
    SO_CONTINUE(void *, LogoCanvas_init_hook);
}

void MenuCanvas_init_patched(){
    l_debug("[MenuCanvas_init] called");
    gameState = 1;
    SO_CONTINUE(void *, MenuCanvas_init_hook);
}

void GameCanvas_init_patched(signed int param1){
    l_debug("[GameCanvas_init] called");
    gameState = 2;
    SO_CONTINUE(void *, GameCanvas_init_hook, param1);
}

int GameUI_searchBag_patched(int param1, int param2){    
    if(param2 == 156){
        l_debug("[GameUI_searchBag] Overriding and returning 1 for param2=156");
        return 1;
    } 
    return SO_CONTINUE(int, GameUI_searchBag_hook, param1, param2);
}

void so_patch(void) {
    Grp_drawWPNImage_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "Grp_drawWPNImage"),
        (uintptr_t)&Grp_drawWPNImage_patched);

    Grp_drawWPNImage2_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "Grp_drawWPNImage2"),
        (uintptr_t)&Grp_drawWPNImage2_patched);

    LogoCanvas_init_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "LogoCanvas_init"),
        (uintptr_t)&LogoCanvas_init_patched);

    MenuCanvas_init_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "MenuCanvas_init"),
        (uintptr_t)&MenuCanvas_init_patched);

    GameCanvas_init_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "GameCanvas_init"),
        (uintptr_t)&GameCanvas_init_patched);

    GameUI_searchBag_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "GameUI_searchBag"),
        (uintptr_t)&GameUI_searchBag_patched);

    drawAGDImageT_hook = hook_addr((uintptr_t)so_symbol(&so_mod, "drawAGDImageT"),
        (uintptr_t)&drawAGDImageT_patched);
}
