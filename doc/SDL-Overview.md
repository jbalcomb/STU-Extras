


Window
Renderer
Texture
Surface
1:1?

Scaling?
...aspect-ratio
...high-dpi

one renderer is *attached* to one window

any texture is copied to one renderer

So, ...
    wrapper for create window and renderer
    wrapper for copy texture to texture, via renderer, with renderer configuration
        SDL_SetRenderTarget(video.renderer, video.texture_upscaled);
        SDL_RenderCopy(video.renderer, video.texture, NULL, NULL);
        SDL_SetRenderTarget(video.renderer, NULL);

multiple windows?
    window id
    events

Update Display:
    texture |-> renderer |-> window
    texture dimensions vs. window dimensions?



Getting a surface compatible with the texture...
    SDL_CreateTextureFromSurface()




## SDL Resources Information

// Get the size of the texture.
int w, h;
SDL_QueryTexture(texture, &format, nullptr, &w, &h);






MoM, MoX, & SDL

320 x 200
6 bits per pixel
4 video buffers
MoM uses the first three as "On", "Off", and "Back", where "back" is always the same, but on and off *flip*.
(MoM also uses EMM to cache a frame for VGAFILEH)

An SDL_Surface is what we need to *draw* on, and then copy to a SDL_Texture for SDL_Renderer().
As luck would have it, SDL happens to support the native pixel format for the IBM-PC VGA DAC.

video_buffer = SDL_CreateRGBSurface(0, XRES, YRES, 8, 0, 0, 0, 0);



get the window from SDL_CreateWindow()
get the pixel format id of the window from SDL_GetWindowPixelFormat()
get the parameters for the pixel format from SDL_PixelFormatEnumToMasks()
get a another new surface with the pixel format parameters from SDL_CreateRGBSurface()

later, copy from the game surface to the window surface

SDL_BlitSurface()
from game surface, to window surface

video_interbuffer is a SDL_Surface that has the pixel format of the platform window

prepare for updating the texture
SDL_LockTexture() ... text, rect, pixels, pitch
    the pitch is the length of one row in bytes

memcpy() the intermediate surface into the intermediate texture

SDL_RenderCopy() the intermediate texture into the final texture

SDL_RenderCopy() the final texture into the window



1) MoX Draw()
2) copy buffer to surface1
3) copy surface1 to surface2
4) copy surface2 to texture2
5) copy texture2 to texture3
6) copy texture3 to window

these are all entire copies? no x,y,w,h
but, ...
somewhere in here they are not the same size?
buffer is 320x200, because VGA 'Mode Y'
surface1? takes video.bufw, video.bufh
surface2? takes video.bufw, video.bufh
texture2? takes video.bufw, video.bufh
texture2? upscales based on video.bufw, video.bufh; also, actualh? NOTE: highdpi means screen pixels != window size

window has its own x,y,w,h
render gets its from window



1oom uses video.blit_rect for the copy from the game's surface to the platform's surface, via SDL_BlitSurface()
video.blit_rect is initialized to {0, 0, video.bufw, video.bufh}, in hw_video_init()
video.bufw,h are set to w,h, passed into hw_video_init()
    lbxview.c:    if (hw_video_init(320, 400)) {
    ui/classic/uiclassic.c:      || hw_video_init(UI_SCREEN_W, UI_SCREEN_H)
ui/classic/uidefs.h:
    #define UI_SCALE_MAX    10
    #define UI_VGA_W    320
    #define UI_VGA_H    200
    #define UI_SCREEN_W ui_screen_w
    #define UI_SCREEN_H ui_screen_h
    uiclassic.c
        int ui_screen_w = 0;
        int ui_screen_h = 0;
        int ui_scale = 0;
        ...
        ui_screen_w = UI_VGA_W * ui_scale;
..."uiscale" is all wrapped up in the cfg/opt code, which is pretty whacktastic code, but it just defaults to 1









How does 1oom get from video.buf[] to video.screen, for the call to 
...in video_render()
    int pitch = video.screen->pitch;
    Uint8 *p = (Uint8 *)video.screen->pixels;
    uint8_t *q = video.buf[bufi];
    for (int y = 0; y < video.bufh; ++y) {
        memcpy(p, q, video.bufw);
        p += pitch;
        q += video.bufw;
    }
...
video.render = video_render;
...
called from hw_video_refresh()
    SDL_LockSurface(video.screen)
    video.render(video.bufi ^ front);
    SDL_UnlockSurface(video.screen);
    video.update();
So, ...
    1oom's video_update() is seemingly equivalent to my Update_Window_Display()



Elsewhere, ...
    if (video.palette_to_set) {
        SDL_SetPaletteColors(video.screen->format->palette, video.color, 0, 256);
        video.palette_to_set = false;
    }

static void video_setpal(uint8_t *pal, int first, int num)
{
    memcpy(&video.pal[first * 3], pal, num * 3);
    for (int i = first; i < (first + num); ++i) {
        video.color[i].r = *pal++ << 2;
        video.color[i].g = *pal++ << 2;
        video.color[i].b = *pal++ << 2;
        video.color[i].a = 255;
    }
    video.palette_to_set = true;
    video_update();
}

## Palette
MoX has palette[1024], which is [768] colors and [256] flags
[768] colors is {R,G,B}, 6 bits per pixel
`<< 2` converts them to 8-bpp format
SDL has struct SDL_Color, which is uint8_t r, g, b, a
a is alpha (for blending) and 255 means full/use it all/dont actually do any blending
...
for the Win PFL, I made it so there was a duplicate palette in XBGR format
which gets updated along with the VGA shadow palette
this breaks the platform API boundary, but also puts the code nearest the task/responsibility
...
for the Win PFL, I made it hand-craft the platform VBB to the *native* format, non-palettized
1oom uses the SDL in-built, palettized 8-bpp RGB surface, then updates its palette by converting the MoX 6-bpp palette
So, ...
    What to do now?
        Decision Criteria?
            support for original SimTex VGA graphics format
            support for modern graphics format(s)?



MoX  Draw() to 320x200x1 VBB, with palette
PFL  copy VBB to SDL game screen->pixels, update screen->format->palette with game palette
...algorithm accomodates arbitrary width and height...
column-wise vs. row-wise?
    iter height, iter width
    iter width, iter height
    copy column
    copy row



SDL_SetPaletteColors(video.screen->format->palette, video.color, 0, 256);








Window
Renderer
Surface
Texture


## SDL Window

video.window = SDL_CreateWindow(0, x, y, w, h, window_flags);



    /* These are (1) the window (or the full screen) that our game is rendered to
       and (2) the renderer that scales the texture (see below) into this window.
    */
    SDL_Window *window;
    SDL_Renderer *renderer;
    /* These are (1) the bufw*bufh*8bpp paletted buffer that we copy the active buffer to,
       (2) the bufw*bufh intermediate buffer of the window/renderer's pixel format that
       we blit the former buffer to,
       (3) the intermediate bufw*bufh texture that we update with the intermediate's buffer data,
       and that we render into another texture (4) which is upscaled by an integer factor
       UPSCALE using "nearest" scaling and which in turn is finally rendered to screen
       using "linear" scaling.
    */
    SDL_Surface *screen;
    SDL_Surface *interbuffer;
    SDL_Texture *texture;
    SDL_Texture *texture_upscaled;


### SDL Window - Pixel Format

uint32_t pixel_format;
pixel_format = SDL_GetWindowPixelFormat(window);

unsigned int rmask, gmask, bmask, amask;
int bpp;

log_message("video.pixel_format: %s\n", SDL_GetPixelFormatName(video.pixel_format));

    if (SDL_PixelFormatEnumToMasks(video.pixel_format, &bpp, &rmask, &gmask, &bmask, &amask))
    {
        log_message("SDL_PixelFormatEnumToMasks(%x) -> %d %x %x %x %x\n", video.pixel_format, bpp, rmask, gmask, bmask, amask);

        if (bpp < 15)
        {
            log_error("Unsupported bits per pixel: %d\n", bpp);
            return -1;
        }

        video.interbuffer = SDL_CreateRGBSurface(0, video.bufw, video.bufh, bpp, rmask, gmask, bmask, amask);

        if (video.interbuffer == NULL)
        {
            log_message("SDL_CreateRGBSurface(): %s\n", SDL_GetError());
            return -1;
        }

        SDL_FillRect(video.interbuffer, NULL, 0);

    }
    else
    {
        log_message("SDL_PixelFormatEnumToMasks(%x): %s\n", video.pixel_format, SDL_GetError());
        return -1;
    }





https://wiki.libsdl.org/SDL2/SDL_GetDisplayMode
https://wiki.libsdl.org/SDL2/SDL_GetDesktopDisplayMode
https://wiki.libsdl.org/SDL2/SDL_GetCurrentDisplayMode

https://wiki.libsdl.org/SDL2/SDL_GetNumVideoDisplays


https://wiki.libsdl.org/SDL2/SDL_SetWindowDisplayMode

https://wiki.libsdl.org/SDL2/SDL_CreateRGBSurface


### SDL_CreateRGBSurface
    Allocate a new RGB surface.
    Syntax
        SDL_Surface* SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
    Function Parameters
        flags	the flags are unused and should be set to 0
        width	the width of the surface
        height	the height of the surface
        depth	the depth of the surface in bits
        Rmask	the red mask for the pixels
        Gmask	the green mask for the pixels
        Bmask	the blue mask for the pixels
        Amask	the alpha mask for the pixels
    Remarks
        If depth is 4 or 8 bits, an empty palette is allocated for the surface.
