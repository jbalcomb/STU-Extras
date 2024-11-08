/*

Startup
Operate
Shutdown

SDL2
ImGui

*/

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <SDL.h>

#include "Configuration.hpp"
#include "STU-SDL.hpp"



// void MoX_SDL_Startup(void);
// void MoX_SDL_Shutdown(void);



void App_Startup(void)
{

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        // return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    g_window = SDL_CreateWindow(sdl_window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, sdl_window_width, sdl_window_height, window_flags);
    if (g_window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        // return -1;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (g_renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        // return 0;
    }
    //SDL_RendererInfo info;
    //SDL_GetRendererInfo(renderer, &info);
    //SDL_Log("Current SDL_Renderer: %s", info.name);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(g_window, g_renderer);
    ImGui_ImplSDLRenderer2_Init(g_renderer);


    uint32_t sdl_window_pixel_format;
    unsigned int rmask, gmask, bmask, amask;
    int bpp;
    sdl_window_pixel_format = SDL_GetWindowPixelFormat(g_window);
    SDL_PixelFormatEnumToMasks(sdl_window_pixel_format, &bpp, &rmask, &gmask, &bmask, &amask);

    g_texture = SDL_CreateTexture(g_renderer, sdl_window_pixel_format, SDL_TEXTUREACCESS_STREAMING, sdl_window_width, sdl_window_height);


    // MoX_SDL_Startup();

}

void App_Shutdown(void)
{
    // MoX_SDL_Shutdown();

    SDL_DestroyTexture(g_texture);

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

}
