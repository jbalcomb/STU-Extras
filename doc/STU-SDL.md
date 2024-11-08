
Platform-Layer
Initialize Platform
Start-Up & Shut-Down
Create & Destroy

SDL_Init() ... ☁ ... SDL_Quit()
◼
▥	
▣	



Application
    Startup
        SDL2
        Windower
        Renderer
        ImGui Context
    Operate
        Mark Time
        Begin Frame
            Update
            Draw
        End Frame
        Render



¿¿¿
...MoX Draw()...
...SDL Update VBB Texture...
???

MWA-GDI:
    `Pump_Paints(); ~~> WndEvnt(); ~~> Update_Window_Display(); StretchDIBits();`
SDL2:
    `... SDL_RenderCopy(); SDL_Render();`

¿ For SDL2, the SDL Texture for SDL_RenderCopy() is equivalent to the Buffer->Memory for StretchDIBits() for MWA ?


if scale_changed
if curr_scale != prev_scale
    (re-) allocate buffer



¿¿¿
...
    ===
        create the art
    ===
        reveal the art
    ===
...
???


MoX_SDL?
not a real thing?
just confused with using MoX outside of MoM?
could/should actually just be in MoX, as an extension/stand-alone?  Mini-MoX?  xMoX  HoX (half/helper)
along with naive upscaling?

SDL_Platform?
create surface
create texture
copy MoX VBB to surface/texture






## Input
Keyboard
Mouse

User_Mouse_Hander()
    mouse position x,y
    mouse button state/status

...translate mouse coordinates...

mouse coordinates for MoX are relative to the game screen in the platform window
...
if mouse_in_mox
    game screen mouse x = platform window mouse x - game screen x

¿ any need for `scale` with the mouse coordinates ?
 







### SDL vs. MWA

MWA                     SDL
                        SDL_Init();
WNDCLASSEX
RegisterClassEx()
AdjustWindowRectEx()    
CreateWindowEx();       SDL_CreateWindow();
ShowWindow()
UpdateWindow()
                        SDL_Quit();

GetMessage();           SDL_WaitEvent();
PeekMessage();          SDL_PollEvent();

SDL_Window * Window
SDL_Texture * Texture
SDL_Renderer * Renderer

SDL_CreateTexture()

SDL_UpdateTexture()
SDL_RenderCopy()
SDL_RenderPresent()

GetClientRect()         SDL_GetWindowSize()

CreateDIBSection()      ~== SDL_CreateTexture()




SDL_WaitEvent()
SDL_PollEvent()
// Event handler
SDL_Event e;


Window
Event Handler

Resize Window

Video Back-Buffer

main()
SDL_Init()
SDL_CreateWindow()
SDL_CreateRenderer()


SDL_Window * Window
SDL_Texture * Texture
SDL_Renderer * Renderer

SDL_CreateTexture()

SDL_UpdateTexture()
SDL_RenderCopy()
SDL_RenderPresent()



SDL_PollEvent()



Memory Allocation

Pixel Format
gTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
¿ OON ?
1:1 pixel format and bytes per pixel?
...vs. RGBA or XBGR in ReMoM?
¿ OON XREF - memory allocation size?



SDL_Texture* newTexture = NULL;
SDL_Surface* loadedSurface = IMG_Load(path.c_str());
newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
SDL_FreeSurface(loadedSurface);
return newTexture;
gTexture = loadTexture("texture.png");



## Event Handling

// Event handler
SDL_Event event;
while (!quit)
{
    while (SDL_PollEvent(&event) != 0)
    {
        if (event.type == SDL_QUIT) { quit = true; }
    }
}

https://wiki.libsdl.org/SDL2/SDL_Event

WM_SIZE

SDL_WINDOWEVENT_SIZE_CHANGED


HMH     Win32ResizeDIBSection()
HMP     ResizeTexture()
ReMoM   Init_Window_Back_Buffer() AKA Resize_Window_Back_Buffer()





C:\STU\devel\STU-Extras\STU-SDL\3PL\SDL2\include\SDL_render.h

/**
 * A structure representing rendering state
 */
struct SDL_Renderer;
typedef struct SDL_Renderer SDL_Renderer;

/**
 * An efficient driver-specific representation of pixel data
 */
struct SDL_Texture;
typedef struct SDL_Texture SDL_Texture;




# References / Education Material
Lazy Foo
Handmade Penguin
