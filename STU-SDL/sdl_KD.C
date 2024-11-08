
#include <stdint.h>

// #include "MoX_BASE.H"
#include "..\ReMoM\src\MoX\MoX_BASE.H"
// #include "Mouse.H"
// #include "..\ReMoM\src\Mouse.H"
// WZD s35p13
void Save_Mouse_State(void);
// WZD s35p14
void Restore_Mouse_State(void);
// MoX_PFL.C
extern int key_pressed;               // win_KD.C  Keyboard_Status(), Read_Key()
extern uint16_t scan_code_char_code;  // win_KD.C  Read_Key()
#include "..\ReMoM\src\Input.H"
// enum ScanCodeCharCode



/*
    Platform-Layer
    PFL Key Code to SimTex Key Code

    e.g., 'F11'  character == 0xFFFF8500

*/
/*
https://wiki.libsdl.org/SDL2/SDL_KeyboardEvent
https://wiki.libsdl.org/SDL2/SDL_Keysym
https://wiki.libsdl.org/SDL2/SDL_Scancode
https://wiki.libsdl.org/SDL2/SDL_Keycode
https://wiki.libsdl.org/SDL2/SDL_Keymod

SDL_Event event;
event.type == SDL_KEYDOWN
event.key.keysym.scancode
event.key.keysym.sym
event.key.keysym.mod

SDL_KeyboardEvent   A structure that contains keyboard button event information.
Data Fields
Uint32      type        the event type; SDL_KEYDOWN or SDL_KEYUP
Uint32      timestamp   timestamp of the event
Uint32      windowID    the window with keyboard focus, if any
Uint8       state       the state of the key; SDL_PRESSED or SDL_RELEASED
Uint8       repeat      non-zero if this is a key repeat
SDL_Keysym  keysym      the SDL_Keysym representing the key that was pressed or released

SDL_Keysym          A structure that contains key information used in key events.
Data Fields
SDL_Scancode    scancode    SDL physical key code; see SDL_Scancode for details
SDL_Keycode     sym         SDL virtual key code; see SDL_Keycode for details
Uint16          mod         current key modifiers; see SDL_Keymod for details
Uint32          unused

*/


uint16_t SDLVK_to_SCCS[128] = {
    /*   0 0x00 */  0x0000,
    /*   1 0x01 */  0x0000,
    /*   2 0x02 */  0x0000,
    /*   3 0x03 */  0x0000,
    /*   4 0x04 */  0x0000,
    /*   5 0x05 */  0x0000,
    /*   6 0x06 */  0x0000,
    /*   7 0x07 */  0x0000,
    /*   8 0x08 */  0x0000,
    /*   9 0x09 */  0x0000,
    /*  10 0x0A */  0x0000,
    /*  11 0x0B */  0x0000,
    /*  12 0x0C */  0x0000,
    /*  13 0x0D */  0x0000,
    /*  14 0x0E */  0x0000,
    /*  15 0x0F */  0x0000,
    /*  16 0x10 */  0x0000,
    /*  17 0x11 */  0x0000,
    /*  18 0x12 */  0x0000,
    /*  19 0x13 */  0x0000,
    /*  20 0x14 */  0x0000,
    /*  21 0x15 */  0x0000,
    /*  22 0x16 */  0x0000,
    /*  23 0x17 */  0x0000,
    /*  24 0x18 */  0x0000,
    /*  25 0x19 */  0x0000,
    /*  26 0x1A */  0x0000,
    /*  27 0x1B */  0x011B,  /* SDLK_ESCAPE */
    /*  28 0x1C */  0x0000,
    /*  29 0x1D */  0x0000,
    /*  30 0x1E */  0x0000,
    /*  31 0x1F */  0x0000,
    /*  32 0x20 */  0x3920,  /* VK_SPACE */
    /*  33 0x21 */  0x0000,
    /*  34 0x22 */  0x0000,
    /*  35 0x23 */  0x0000,
    /*  36 0x24 */  0x0000,
    /*  37 0x25 */  SCCC_LEFT,  /* VK_LEFT */
    /*  38 0x26 */  SCCC_UP, /* VK_UP */
    /*  39 0x27 */  SCCC_RIGHT, /* VK_RIGHT */
    /*  40 0x28 */  SCCC_DOWN, /* VK_DOWN */
    /*  41 0x29 */  0x0000,
    /*  42 0x2A */  0x0000,
    /*  43 0x2B */  0x0000,
    /*  44 0x2C */  0x0000,
    /*  45 0x2D */  0x0000,
    /*  46 0x2E */  0x0000,
    /*  47 0x2F */  0x0000,
    /*  48 0x30 */  0x0B30,  /* VK_0 */
    /*  49 0x31 */  0x0231,  /* VK_1 */
    /*  50 0x32 */  0x0332,  /* VK_2 */
    /*  51 0x33 */  0x0433,  /* VK_3 */
    /*  52 0x34 */  0x0534,  /* VK_4 */
    /*  53 0x35 */  0x0635,  /* VK_5 */
    /*  54 0x36 */  0x0736,  /* VK_6 */
    /*  55 0x37 */  0x0837,  /* VK_7 */
    /*  56 0x38 */  0x0938,  /* VK_8 */
    /*  57 0x39 */  0x0A39,  /* VK_9 */
    /*  58 0x3A */  0x0000,  /* unassigned */
    /*  59 0x3B */  0x0000,  /* unassigned */
    /*  60 0x3C */  0x0000,  /* unassigned */
    /*  61 0x3D */  0x0000,  /* unassigned */
    /*  62 0x3E */  0x0000,  /* unassigned */
    /*  63 0x3F */  0x0000,  /* unassigned */
    /*  64 0x40 */  0x0000,  /* unassigned */
    /*  65 0x41 */  0x1E61,  /* VK_A */
    /*  66 0x42 */  0x3062,  /* VK_B */
    /*  67 0x43 */  0x2E63,  /* VK_C */
    /*  68 0x44 */  0x2064,  /* VK_D */
    /*  69 0x45 */  0x1265,  /* VK_E */
    /*  70 0x46 */  0x2166,  /* VK_F */
    /*  71 0x47 */  0x2267,  /* VK_G */
    /*  72 0x48 */  0x2368,  /* VK_H */
    /*  73 0x49 */  0x1769,  /* VK_I */
    /*  74 0x4A */  0x246A,  /* VK_J */
    /*  75 0x4B */  0x256B,  /* VK_K */
    /*  76 0x4C */  0x266C,  /* VK_L */
    /*  77 0x4D */  0x326D,  /* VK_M */
    /*  78 0x4E */  0x316E,  /* VK_N */
    /*  79 0x4F */  0x186F,  /* VK_O */
    /*  80 0x50 */  0x1970,  /* VK_P */
    /*  81 0x51 */  0x1071,  /* VK_Q */
    /*  82 0x52 */  0x1372,  /* VK_R */
    /*  83 0x53 */  0x1F73,  /* VK_S */
    /*  84 0x54 */  0x1474,  /* VK_T */
    /*  85 0x55 */  0x1675,  /* VK_U */
    /*  86 0x56 */  0x2F76,  /* VK_V */
    /*  87 0x57 */  0x1177,  /* VK_W */
    /*  88 0x58 */  0x2D78,  /* VK_X */
    /*  89 0x59 */  0x1579,  /* VK_Y */
    /*  90 0x5A */  0x2C7A,  /* VK_Z */
    /*  91 0x5B */  0x0000,
    /*  92 0x5C */  0x0000,
    /*  93 0x5D */  0x0000,
    /*  94 0x5E */  0x0000,
    /*  95 0x5F */  0x0000,
    /*  96 0x60 */  0x0000,  /* VK_NUMPAD0 */
    /*  97 0x61 */  SCCC_KP_1,  /* VK_NUMPAD1 */
    /*  98 0x62 */  SCCC_KP_2,  /* VK_NUMPAD2 */
    /*  99 0x63 */  SCCC_KP_3,  /* VK_NUMPAD3 */
    /* 100 0x64 */  SCCC_KP_4,  /* VK_NUMPAD4 */
    /* 101 0x65 */  0x0000,  // SCCC_KP_5,  /* VK_NUMPAD5 */
    /* 102 0x66 */  SCCC_KP_6,  /* VK_NUMPAD6 */
    /* 103 0x67 */  SCCC_KP_7,  /* VK_NUMPAD7 */
    /* 104 0x68 */  SCCC_KP_8,  /* VK_NUMPAD8 */
    /* 105 0x69 */  SCCC_KP_9,  /* VK_NUMPAD9 */
    /* 106 0x6A */  0x0000,
    /* 107 0x6B */  0x0000,
    /* 108 0x6C */  0x0000,
    /* 109 0x6D */  0x0000,
    /* 110 0x6E */  0x0000,
    /* 111 0x6F */  0x0000,
    /* 112 0x70 */  0x0000,
    /* 113 0x71 */  0x0000,
    /* 114 0x72 */  0x0000,
    /* 115 0x73 */  0x0000,
    /* 116 0x74 */  0x0000,
    /* 117 0x75 */  0x0000,
    /* 118 0x76 */  0x0000,
    /* 119 0x77 */  0x0000,
    /* 120 0x78 */  0x0000,
    /* 121 0x79 */  0x0000,
    /* 122 0x7A */  0x0000,
    /* 123 0x7B */  0x0000,
    /* 124 0x7C */  0x0000,
    /* 125 0x7D */  0x0000,
    /* 126 0x7E */  0x0000,
    /* 127 0x7F */  0x0000
};



// WZD s35p14
// keystroke available: {0,1} {ST_FALSE,ST_TRUE}
int16_t Keyboard_Status(void)
{
    int16_t keyboard_status;

    Save_Mouse_State();

    // Pump_Events();

    if (key_pressed == ST_TRUE)
    {
        keyboard_status = ST_TRUE;
    }
    else
    {
        keyboard_status = ST_FALSE;
    }

    Restore_Mouse_State();

    return keyboard_status;
}


// WZD s35p15
uint8_t Read_Key(void)
{
    int8_t return_key;

    key_pressed = ST_FALSE;

    switch (scan_code_char_code)
    {
    case SCCC_ESC: { return_key = ST_KEY_ESCAPE; } break;

    case SCCC_F10: { return_key = ST_KEY_F10; } break;

    case SCCC_A: { return_key = 'a'; } break;
    case SCCC_B: { return_key = 'b'; } break;
    case SCCC_C: { return_key = 'c'; } break;
    case SCCC_D: { return_key = 'd'; } break;
    case SCCC_E: { return_key = 'e'; } break;
    case SCCC_F: { return_key = 'f'; } break;
    case SCCC_G: { return_key = 'g'; } break;
    case SCCC_H: { return_key = 'h'; } break;
    case SCCC_I: { return_key = 'i'; } break;
    case SCCC_J: { return_key = 'j'; } break;
    case SCCC_K: { return_key = 'k'; } break;
    case SCCC_L: { return_key = 'l'; } break;
    case SCCC_M: { return_key = 'm'; } break;
    case SCCC_N: { return_key = 'n'; } break;
    case SCCC_O: { return_key = 'o'; } break;
    case SCCC_P: { return_key = 'p'; } break;
    case SCCC_Q: { return_key = 'q'; } break;
    case SCCC_R: { return_key = 'r'; } break;
    case SCCC_S: { return_key = 's'; } break;
    case SCCC_T: { return_key = 't'; } break;
    case SCCC_U: { return_key = 'u'; } break;
    case SCCC_V: { return_key = 'v'; } break;
    case SCCC_W: { return_key = 'w'; } break;
    case SCCC_X: { return_key = 'x'; } break;
    case SCCC_Y: { return_key = 'y'; } break;
    case SCCC_Z: { return_key = 'z'; } break;

        // BackSpace   0E08
        // Del         5300
    case SCCC_KP_2: { return_key = ST_KEY_DOWN; } break;
    case SCCC_DOWN: { return_key = ST_KEY_DOWN; } break;
    case SCCC_KP_1: { return_key = ST_KEY_LEFTDOWN; } break;
    case SCCC_DOWNLEFT: { return_key = ST_KEY_LEFTDOWN; } break;
        // Enter       1C0D
        // Esc         011B
    case SCCC_KP_7: { return_key = ST_KEY_LEFTUP; } break;
    case SCCC_UPLEFT: { return_key = ST_KEY_LEFTUP; } break;
        // Ins         5200
        // Keypad 5        
        // Keypad *    372A
        // Keypad -    4A2D
        // Keypad +    4E2B
        // Keypad /    352F
    case SCCC_KP_4: { return_key = ST_KEY_LEFT; } break;
    case SCCC_LEFT: { return_key = ST_KEY_LEFT; } break;
    case SCCC_KP_3: { return_key = ST_KEY_RIGHTDOWN; } break;
    case SCCC_DOWNRIGHT: { return_key = ST_KEY_RIGHTDOWN; } break;
    case SCCC_KP_9: { return_key = ST_KEY_RIGHTUP; } break;
    case SCCC_UPRIGHT: { return_key = ST_KEY_RIGHTUP; } break;
        // PrtSc           
    case SCCC_KP_6: { return_key = ST_KEY_RIGHT; } break;
    case SCCC_RIGHT: { return_key = ST_KEY_RIGHT; } break;
        // SpaceBar    3920
        // Tab         0F09
    case SCCC_KP_8: { return_key = ST_KEY_UP; } break;
    case SCCC_UP: { return_key = ST_KEY_UP; } break;

    default: { return_key = ST_KEY_OVERRUN; }
    }

    return return_key;
}
