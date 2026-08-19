#include "utils/init.h"
#include "utils/glutil.h"

#include <psp2/kernel/threadmgr.h>

#include <falso_jni/FalsoJNI.h>
#include <so_util/so_util.h>

#include "reimpl/controls.h"

#include "audio.h"

int _newlib_heap_size_user = 256 * 1024 * 1024;

#ifdef USE_SCELIBC_IO
int sceLibcHeapSize = 4 * 1024 * 1024;
#endif

extern JNIEnv jni; 
so_module so_mod;

extern int settings_capframerate;
extern int settings_highres;

int gameWidth = 320;
int gameHeight = 240;

int screenWidth = 960;
int screenHeight = 544;

typedef enum {
    DPAD_NONE,
    DPAD_LEFT,
    DPAD_RIGHT,
    DPAD_UP,
    DPAD_DOWN
} virtual_buttons;

static virtual_buttons current_dpad_direction = DPAD_NONE;

int gameState = 0; //0 = logo, 1 = menu, 2 = game

void (*jniTouch)(void*, int, int, int, int);

void (*GC_mKeyPressed)(int);
void (*GC_mKeyReleased)(int);
void (*MC_mKeyPressed)(int);
void (*MC_mKeyReleased)(int);
void (*LC_mKeyPressed)(int);

int main() {
    soloader_init_all();

    if(settings_highres) {
        gameWidth = 480;
        gameHeight = 320;
    }

    size_t framebuffer_size = gameWidth * gameHeight * 2;
    jbyteArray screenBuf = jni->NewByteArray(jni, framebuffer_size);

    void (*initGame)(void*, void*, int, int, int, int, int, int, char) = (void *)so_symbol(&so_mod, "Java_game_destiniaeng_GameThread_initGame");
    void (*jniRun)(void*, int, int) = (void *)so_symbol(&so_mod, "Java_game_destiniaeng_GameThread_jniRun");

    jniTouch = (void *)so_symbol(&so_mod, "Java_game_destiniaeng_GameThread_jniTouch");
    GC_mKeyPressed = (void *)so_symbol(&so_mod, "GameCanvas_mKeyPressed");
    GC_mKeyReleased = (void *)so_symbol(&so_mod, "GameCanvas_mKeyReleased");
    
    MC_mKeyPressed = (void *)so_symbol(&so_mod, "MenuCanvas_mKeyPressed");
    MC_mKeyReleased = (void *)so_symbol(&so_mod, "MenuCanvas_mKeyReleased");

    LC_mKeyPressed = (void *)so_symbol(&so_mod, "LogoCanvas_mKeyPressed");

    initGame(&jni, NULL, 0, 0, 0, 0, gameWidth, gameHeight, 0);

    audio_init();    
    gl_init();

    audio_preload(-1);

    gl_init_screen_texture(gameWidth, gameHeight);
    
    if(settings_capframerate) {
        eglSwapInterval(0, 2);
    }

    while (1) {
        controls_poll();

        jniRun(&jni, 0, screenBuf);

        jbyte *pixels = jni->GetByteArrayElements(&jni, screenBuf, NULL);
        gl_present_framebuffer(pixels);
        jni->ReleaseByteArrayElements(&jni, screenBuf, pixels, JNI_ABORT);

        gl_swap();
}
    sceKernelExitDeleteThread(0);
}

void controls_handler_key(int32_t keycode, ControlsAction action) {
    if (action == CONTROLS_ACTION_DOWN) {
        switch(keycode){
            case AKEYCODE_DPAD_DOWN:
                if(gameState == 1){
                    MC_mKeyPressed(9);
                } else if(gameState == 2){
                    GC_mKeyPressed(9);
                }
                break;
            case AKEYCODE_DPAD_LEFT:
                if(gameState == 1){
                    MC_mKeyPressed(5);
                } else if(gameState == 2){
                    GC_mKeyPressed(5);
                }
                break;
            case AKEYCODE_DPAD_RIGHT:
                if(gameState == 1){
                    MC_mKeyPressed(7);
                } else if(gameState == 2){
                    GC_mKeyPressed(7);
                }
                break;
            case AKEYCODE_DPAD_UP:
                if(gameState == 1){
                    MC_mKeyPressed(3);
                } else if(gameState == 2){
                    GC_mKeyPressed(3);
                }
                break;
            case AKEYCODE_BUTTON_A:
                if(gameState == 1){
                    MC_mKeyPressed(17);
                } else if(gameState == 2){
                    GC_mKeyPressed(17);
                } else if(gameState == 0){
                    LC_mKeyPressed(17);
                }
                break;
            case AKEYCODE_BUTTON_B:
                if(gameState == 1){
                    MC_mKeyPressed(10);
                } else if(gameState == 2){
                    GC_mKeyPressed(10);
                }
                break;
            case AKEYCODE_BUTTON_Y:
                if(gameState == 1){
                    MC_mKeyPressed(27);
                } else if(gameState == 2){
                    GC_mKeyPressed(27);
                }
                break;
            case AKEYCODE_BUTTON_START:
                if(gameState == 1){
                    MC_mKeyPressed(1);
                } else if(gameState == 2){
                    GC_mKeyPressed(1);
                }
                break;
            case AKEYCODE_BUTTON_R1:
                if(gameState == 1){
                    MC_mKeyPressed(12);
                } else if(gameState == 2){
                    if(settings_highres){
                        controls_handler_touch(0, 947.0, 291.0, CONTROLS_ACTION_DOWN);
                    } else {
                        controls_handler_touch(0, 911.5, 225.0, CONTROLS_ACTION_DOWN);
                    }
                }
                break;
        }
    }

    if(action == CONTROLS_ACTION_UP){
        switch(keycode){
            case AKEYCODE_DPAD_DOWN:
                if(gameState == 1){
                    MC_mKeyReleased(9);
                } else if(gameState == 2){
                    GC_mKeyReleased(9);
                }
                break;
            case AKEYCODE_DPAD_LEFT:
                if(gameState == 1){
                    MC_mKeyReleased(5);
                } else if(gameState == 2){
                    GC_mKeyReleased(5);
                }
                break;
            case AKEYCODE_DPAD_RIGHT:
                if(gameState == 1){
                    MC_mKeyReleased(7);
                } else if(gameState == 2){
                    GC_mKeyReleased(7);
                }
                break;
            case AKEYCODE_DPAD_UP:
                if(gameState == 1){
                    MC_mKeyReleased(3);
                } else if(gameState == 2){
                    GC_mKeyReleased(3);
                }
                break;
            case AKEYCODE_BUTTON_A:
                if(gameState == 1){
                    MC_mKeyReleased(17);
                } else if(gameState == 2){
                    GC_mKeyReleased(17);
                }
                break;
            case AKEYCODE_BUTTON_B:
                if(gameState == 1){
                    MC_mKeyReleased(10);
                } else if(gameState == 2){
                    GC_mKeyReleased(10);
                }
                break;
            case AKEYCODE_BUTTON_Y:
                if(gameState == 1){
                    MC_mKeyReleased(27);
                } else if(gameState == 2){
                    GC_mKeyReleased(27);
                }
                break;
            case AKEYCODE_BUTTON_START:
                if(gameState == 1){
                    MC_mKeyReleased(1);
                } else if(gameState == 2){
                    GC_mKeyReleased(1);
                }
                break;
        }
    }

}

void controls_handler_touch(int32_t id, float x, float y, ControlsAction action) {
    float xx = x * ((float)gameWidth / (float)screenWidth);
    float yy = y * ((float)gameHeight / (float)screenHeight);

    switch(action){
        case CONTROLS_ACTION_UP: 
            jniTouch(&jni, 0, (int)xx, (int)yy, 1);
            break;
        case CONTROLS_ACTION_DOWN: 
            l_debug("[controls_handler_touch] Touch down at (%f, %f) -> (%d, %d)", x, y, (int)xx, (int)yy);
            jniTouch(&jni, 0, (int)xx, (int)yy, 0);
            break;
        case CONTROLS_ACTION_MOVE: 
            jniTouch(&jni, 0, (int)xx, (int)yy, 2);
            break;
    }
}

void controls_handler_analog(ControlsStickId which, float x, float y, ControlsAction action) {
    
    if (which == CONTROLS_STICK_LEFT) {
        virtual_buttons new_dpad_direction = DPAD_NONE;

        if (action != CONTROLS_ACTION_UP) {
            if (fabsf(x) > fabsf(y)) {
                if (x < -0.5f)      new_dpad_direction = DPAD_LEFT;
                else if (x > 0.5f)  new_dpad_direction = DPAD_RIGHT;
            } else {
                if (y < -0.5f)      new_dpad_direction = DPAD_UP;
                else if (y > 0.5f)  new_dpad_direction = DPAD_DOWN;
            }
        }

        if (new_dpad_direction != current_dpad_direction) {
            if (current_dpad_direction != DPAD_NONE) {
                switch (current_dpad_direction) {
                    case DPAD_LEFT:
                        if (gameState == 1) { 
                            MC_mKeyReleased(5);
                        } else if (gameState == 2) {
                            GC_mKeyReleased(5);
                        }
                        break;
                    case DPAD_RIGHT:
                        if (gameState == 1) { 
                            MC_mKeyReleased(7);
                        } else if (gameState == 2) {
                            GC_mKeyReleased(7);
                        }
                        break;
                    case DPAD_UP:
                        if (gameState == 1) { 
                            MC_mKeyReleased(3);
                        } else if (gameState == 2) {
                            GC_mKeyReleased(3);
                        }
                        break;
                    case DPAD_DOWN:
                        if (gameState == 1) { 
                            MC_mKeyReleased(9);
                        } else if (gameState == 2) {
                            GC_mKeyReleased(9);
                        }
                        break;
                }
            }

            if (new_dpad_direction != DPAD_NONE) {
                switch (new_dpad_direction) {
                    case DPAD_LEFT:
                        if (gameState == 1) { 
                            MC_mKeyPressed(5);
                        } else if (gameState == 2) {
                            GC_mKeyPressed(5);
                        }
                        break;
                    case DPAD_RIGHT:
                        if (gameState == 1) { 
                            MC_mKeyPressed(7);
                        } else if (gameState == 2) {
                            GC_mKeyPressed(7);
                        }
                        break;
                    case DPAD_UP:
                        if (gameState == 1) { 
                            MC_mKeyPressed(3);
                        } else if (gameState == 2) {
                            GC_mKeyPressed(3);
                        }
                        break;
                    case DPAD_DOWN:
                        if (gameState == 1) { 
                            MC_mKeyPressed(9);
                        } else if (gameState == 2) {
                            GC_mKeyPressed(9);
                        }
                        break;
                }
            }

            current_dpad_direction = new_dpad_direction;
        }
    }

    if (which == CONTROLS_STICK_RIGHT) {

        if (action != CONTROLS_ACTION_UP) {
            if (fabsf(x) > fabsf(y)) {
                if (x < -0.5f) {
                    GC_mKeyPressed(2);
                }  else if (x > 0.5f) {
                    GC_mKeyPressed(11);
                }
            } else {
                if (y < -0.5f) {
                    GC_mKeyPressed(4);
                } else if (y > 0.5f) {
                    GC_mKeyPressed(8);
                }
            }
        }

    }
}