


Application  <<~~~>>  Game

SDL  <<~~~>>  ImGui

SDL  <<~~~>>  MoX

Application
    Process Events
        Window Events
        Input Events
            if !paused then Game Input Events

¿ ctor / dtor ?
    SDL_Init()
    ...
    SDL_Quit()


Scion_2D declares the instance of the Application class as `static`
...means that variable is shared by all instances of that class
...singleton?
But, Chili does not...
also, Chili does `App{}.Go()` ... what's with the braces / curly brackets?

() vs {} when constructing objects : r/cpp - Reddit

Since C++11 we have uniform brace-init syntax which lets initialize objects using {} instead of ().
I have read that as of now, {} is recommended because it disallows narrowing convertions and can be applied to types without any declared constructors.

https://learn.microsoft.com/en-us/cpp/cpp/initializing-classes-and-structs-without-constructors-cpp?view=msvc-170
...
If a type has a default constructor, either implicitly or explicitly declared, you can use brace initialization with empty braces to invoke it.

So, ...

declare
define
implement
instantiate
...Class
"Singleton"


### T_00
bare minimum - does ~nothing
#include <SDL.h> int main(int, char**) { SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO); SDL_Quit(); return 0; }

### T_01



...
...
...
SDL_CreateWindow()
SDL_CreateRenderer()













Project Structure

Application vs. ImGui vs. SDL



ImGui > SDL



Application Framework

...at some point, SDL_RenderCopy() the texture from the application to the renderer for ImGui


Layers (?)

ImGui+SDL
    window
    renderer
    where's the texture?
    what is ImGui rendering?

MoX needs to get at a texture for the SDL_RenderCopy() in the ImGui+SDL layer...
pretty sure that should get passed in
and, there should be an application layer startup and shutdown



¿ Update_Window_Display_2x_XBGR() ?
    Where did that go?
    When did I stop using?
    Why did I stop using it?
    ...
    Video2 is still allocating video_page_buffer_2x and video_page_buffer_2x_XBGR
    ...
    C:\STU\devel\__ReMoM__20240117\src\PoC_MsWin.cpp



## Coords & Dims



w,h MoX VBB  internal  e.g., Draw()
...currently exists as `screen_pixel_width, screen_pixel_height`

...naive upscale... ~!= PFL

w,h for surface, texture

w,h for window



    x,y,w,h for texture for SDL_RenderCopy()
SDL
    w,h for mox 6bpp surface
    w,h for mox 32bpp surface
MoX
    VBB
        {N/A, N/A, 320, 200}
    Surface
        {N/A, N/A, 320, 200}
        {N/A, N/A, 320, 200}
    Texture
        {center_x, 20, 320, 200}

MoX/MoM screen width,height is base VGA 'Mode Y' 320x200
and, there never changes?
except, I have the naive upscale for 2x?
...that happens in-between MoX and PFL, in a currently undefined layer...
so, MoX/MoM is 320x200
and, PFL has a 'scale' of 2x
...`enable plaform-layer naive scaling = true`...`platform layer naive scaling factor = 2`
therefore, ...
    SDL-MoX needs w,h from PFL

for SDL_BlitSurface(), x,y,w, are always the same?


¿ contingent on scaling on the MoM/MoX side ?


### Mouse / Cusor X,Y
x,y in global space
x,y in display
x,y in window
x,y in game screen
x,y in MoX


max x,y / mouse max x,y
((320 * mox_scale) - 1)
((200 * mox_scale) - 1)
((mox_screen_pixel_width  * mox_scale) - 1)
((mox_screen_pixel_height * mox_scale) - 1)

    



Startup
    SDL, ImGui, Application


Operate
    Main Loop
    ImGui Frame
    Render - ImGui, SDL


Shutdown
    Application, ImGui, SDL





## Startup & Shutdown  (AKA Create & Destroy)

    SDL_Init(); SDL_CreateWindow(); SDL_CreateRenderer();

    ¿ ImGui Context ?
    ImGui IO
    ImGui Impl - Platform/Backend



### Example - Walnut - Cherno

https://github.com/StudioCherno/Walnut/

Application.cpp
layer->OnUpdate(m_TimeStep);
layer->OnUIRender();

Application::Run()
    layer->OnUpdate(m_TimeStep);
    ...recreate surface, texture, swap-chain, etc.
    ImGui::NewFrame();
        ...ImGui::BeginMenuBar()
        layer->OnUIRender();
    ImGui::End();

