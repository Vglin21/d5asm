#ifndef _D5_DISPLAY_H_
#define _D5_DISPLAY_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef _CUSTOM_TYPES_
#define _CUSTOM_TYPES_

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;

#endif

typedef enum {
    DISPLAY_TYPE_CRT,
    DISPLAY_TYPE_LCD
} DisplayType;

typedef enum {
    DISPK_A = 4,
    DISPK_B = 5,
    DISPK_C = 6,
    DISPK_D = 7,
    DISPK_E = 8,
    DISPK_F = 9,
    DISPK_G = 10,
    DISPK_H = 11,
    DISPK_I = 12,
    DISPK_J = 13,
    DISPK_K = 14,
    DISPK_L = 15,
    DISPK_M = 16,
    DISPK_N = 17,
    DISPK_O = 18,
    DISPK_P = 19,
    DISPK_Q = 20,
    DISPK_R = 21,
    DISPK_S = 22,
    DISPK_T = 23,
    DISPK_U = 24,
    DISPK_V = 25,
    DISPK_W = 26,
    DISPK_X = 27,
    DISPK_Y = 28,
    DISPK_Z = 29,

    DISPK_RETURN = 40,
    DISPK_ESCAPE = 41,
    DISPK_BACKSPACE = 42,
    DISPK_TAB = 43,
    DISPK_SPACE = 44,

    DISPK_MINUS = 45,
    DISPK_EQUALS = 46,
    DISPK_LBRACKET = 47,
    DISPK_RBRACKET = 48,
    DISPK_BACKSLASH = 49,

    DISPK_SEMICOLON = 51,
    DISPK_APOSTROPHE = 52,
    DISPK_GRAVE = 53,
    DISPK_COMMA = 54,
    DISPK_PERIOD = 55,
    DISPK_SLASH = 56,

    DISPK_CAPSLOCK = 57,

    DISPK_F1 = 58,
    DISPK_F2 = 59,
    DISPK_F3 = 60,
    DISPK_F4 = 61,
    DISPK_F5 = 62,
    DISPK_F6 = 63,
    DISPK_F7 = 64,
    DISPK_F8 = 65,
    DISPK_F9 = 66,
    DISPK_F10 = 67,
    DISPK_F11 = 68,
    DISPK_F12 = 69,

    DISPK_PRINTSCREEN = 70,
    DISPK_SCROLLLOCK = 71,
    DISPK_PAUSE = 72,
    DISPK_INSERT = 73,
    DISPK_HOME = 74,
    DISPK_PAGEUP = 75,
    DISPK_DELETE = 76,
    DISPK_END = 77,
    DISPK_PAGEDOWN = 78,
    
    DISPK_RIGHT = 79,
    DISPK_LEFT = 80,
    DISPK_DOWN = 81,
    DISPK_UP = 82,

    DISPK_NUMLOCK = 83,
    
    DISPK_KP_DIVIDE = 84,
    DISPK_KP_MULTIPLY = 85,
    DISPK_KP_MINUS = 86,
    DISPK_KP_PLUS = 87,
    DISPK_KP_ENTER = 88,
    DISPK_KP_1 = 89,
    DISPK_KP_2 = 90,
    DISPK_KP_3 = 91,
    DISPK_KP_4 = 92,
    DISPK_KP_5 = 93,
    DISPK_KP_6 = 94,
    DISPK_KP_7 = 95,
    DISPK_KP_8 = 96,
    DISPK_KP_9 = 97,
    DISPK_KP_0 = 98,
    DISPK_KP_PERIOD = 99,

    DISPK_LCTRL = 224,
    DISPK_LSHIFT = 225,
    DISPK_LALT = 226,
    DISPK_RCTRL = 228,
    DISPK_RSHIFT = 229,
    DISPK_RALT = 230
} DisplayKey;

typedef enum {
    DISP_GAMEPAD_X = 1,
    DISP_GAMEPAD_A,
    DISP_GAMEPAD_B,
    DISP_GAMEPAD_Y,
    DISP_GAMEPAD_L1,
    DISP_GAMEPAD_R1,
    DISP_GAMEPAD_L2,
    DISP_GAMEPAD_R2,
    DISP_GAMEPAD_SELECT, 
    DISP_GAMEPAD_ENTER,
    DISP_GAMEPAD_L3,
    DISP_GAMEPAD_R3,
    DISP_GAMEPAD_UP,
    DISP_GAMEPAD_DOWN,
    DISP_GAMEPAD_LEFT,
    DISP_GAMEPAD_RIGHT,
} DisplayGamepadButton;

extern bool display_turn_on(const char *title, dword width, dword height, DisplayType type);
extern void display_turn_off();
extern void display_close();

extern bool display_should_close();
extern bool display_is_key_pressed(DisplayKey key);
extern bool display_is_gamepad_pressed(DisplayGamepadButton button);
extern bool display_is_frame_active();
extern bool display_is_fullscreen();
extern bool display_is_gamepad_connected();
extern bool display_is_resizable();

extern bool display_set_signal_size(word width, word height, word hblank, word vblank);
extern void display_set_fps(double fps);
extern void display_set_aspect_ratio(double aspect_ratio);
extern void display_set_window_size(dword width, dword height);
extern void display_set_fullscreen(bool fullscreen);
extern void display_set_resizable(bool resizable);

extern void display_draw_pixel(dword color);
extern void display_draw_pixels(dword color, dword count);

extern void display_update();

#endif