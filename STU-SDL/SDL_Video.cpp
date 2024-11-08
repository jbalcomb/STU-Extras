
/*



*/

#include "SDL.h"



/**
 * The bits of this structure can be directly reinterpreted as an integer-packed
 * color which uses the SDL_PIXELFORMAT_RGBA32 format (SDL_PIXELFORMAT_ABGR8888
 * on little-endian systems and SDL_PIXELFORMAT_RGBA8888 on big-endian systems).
 */
typedef struct MOX_Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} MOX_Color;
#define MOX_Colour MOX_Color

typedef struct MOX_Palette
{
    int ncolors;
    MOX_Color* colors;
    uint32_t version;
    int refcount;
} MOX_Palette;



/* double buffering + 2 aux buffers */
#define NUM_VIDEOBUF    4

#define RESIZE_DELAY 500

static struct sdl_video_s {
    /* These are (1) the window (or the full screen) that our game is rendered to
       and (2) the renderer that scales the texture (see below) into this window.
    */
    SDL_Window* window;
    SDL_Renderer* renderer;
    /* These are (1) the bufw*bufh*8bpp paletted buffer that we copy the active buffer to,
       (2) the bufw*bufh intermediate buffer of the window/renderer's pixel format that
       we blit the former buffer to,
       (3) the intermediate bufw*bufh texture that we update with the intermediate's buffer data,
       and that we render into another texture (4) which is upscaled by an integer factor
       UPSCALE using "nearest" scaling and which in turn is finally rendered to screen
       using "linear" scaling.
    */
    SDL_Surface* screen;
    SDL_Surface* interbuffer;
    SDL_Texture* texture;
    SDL_Texture* texture_upscaled;

    SDL_Rect blit_rect;
    uint32_t pixel_format;

    SDL_Surface* icon;
    SDL_Palette* iconpal;

    /* SDL display number on which to run. */
    int display;

    int w_upscale, h_upscale;
    int actualh;

    bool need_resize;
    int last_resize_time;

    void (*render)(int bufi);
    void (*update)(void);
    void (*setpal)(uint8_t* pal, int first, int num);

    /* buffers used by UI */
    uint8_t* buf[NUM_VIDEOBUF];
    int bufw;
    int bufh;
    int bufi;

    /* palette as set by UI, 6bpp */
    uint8_t pal[256 * 3];
    /* palette as used by SDL */
    SDL_Color color[256];
    bool palette_to_set;
    bool flag_screenshot;

} video = { 0 };
