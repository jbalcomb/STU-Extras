#include <Windows.h>
#include <debugapi.h>
#pragma comment( lib, "Winmm.lib" )

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <stdio.h>
#include <SDL.h>

#include "Configuration.hpp"
#include "STU-SDL.hpp"

void Init_MoX(void);



void Update_MoX_Texture(void);
void ImGui_Windows(void);

extern "C" void User_Mouse_Handler(int16_t buttons, int16_t mx, int16_t my);



bool show_demo_window = true;
bool show_options_window = true;
bool show_framerate_window = true;
bool show_inputs_window = true;



int game_loop_count = 0;
int video_frame_count = 0;
bool game_pause = false;


SDL_Window* g_window;
SDL_Renderer* g_renderer;
SDL_Texture* g_texture;
uint32_t sdl_window_pixel_format;
int sdl_screen_pixel_width = 320;
int sdl_screen_pixel_height = 200;
int sdl_window_center_x;
int sdl_window_center_y;
SDL_Rect sdl_window_rect;

int mouse_window_x = 0;
int mouse_window_y = 0;
unsigned int mouse_button_state = 0;
int mouse_button_left = 0;
int mouse_button_middle = 0;
int mouse_button_right = 0;
int mouse_button_x1;
int mouse_button_x2;
bool mouse_in_mox_screen = false;
bool lock_mouse_in_mox_screen = false;
int mox_mouse_position_x = -1;
int mox_mouse_position_y = -1;

SDL_Surface* g_mox_vbb_surface1;
SDL_Surface* g_mox_vbb_surface2;
SDL_Texture* g_mox_vbb_texture;
int mox_screen_pixel_width = 320;
int mox_screen_pixel_height = 200;
SDL_Rect mox_texture_rect = { 10, 10, mox_screen_pixel_width, mox_screen_pixel_height };
int mox_scale = 1;

typedef unsigned char * byte_ptr;
extern "C" byte_ptr p_Palette;
extern "C" int16_t draw_page_num;
extern "C" uint8_t * video_page_buffer[];

// MoX_PFL.C
extern "C" int key_pressed;               // win_KD.C  Keyboard_Status(), Read_Key()
extern "C" uint16_t scan_code_char_code;  // win_KD.C  Read_Key()


char OutputDebugBuffer[4096];


int quit_flag = 0;




void Update_MoX_Texture(void)
{
   
    // int pitch = g_mox_vbb_surface1->pitch;
    uint8_t* dst = (uint8_t*)g_mox_vbb_surface1->pixels;
    uint8_t* src = video_page_buffer[(1 - draw_page_num)];
    int width = mox_screen_pixel_width;
    int height = mox_screen_pixel_height;
    int itr_width;
    int itr_height;
    uint8_t pixel;

    for (itr_height = 0; itr_height < height; itr_height++)
    {
        for (itr_width = 0; itr_width < width; itr_width++)
        {
            pixel = *(src + (itr_height * width) + itr_width);
            *dst++ = pixel;
        }
    }

    SDL_Color mox_sdl_surface1_palette[256];
    
    uint8_t* pal;
    int itr_pal;
    pal = (uint8_t*)p_Palette;
    for (itr_pal = 0; itr_pal < 256; itr_pal++)
    {
        mox_sdl_surface1_palette[itr_pal].r = *pal++ << 2;
        mox_sdl_surface1_palette[itr_pal].g = *pal++ << 2;
        mox_sdl_surface1_palette[itr_pal].b = *pal++ << 2;
        mox_sdl_surface1_palette[itr_pal].a = 255;
    }

    SDL_SetPaletteColors(g_mox_vbb_surface1->format->palette, mox_sdl_surface1_palette, 0, 256);

    SDL_Rect blit_rect;
    blit_rect.x = 0;
    blit_rect.y = 0;
    blit_rect.w = mox_screen_pixel_width;
    blit_rect.h = mox_screen_pixel_height;
    SDL_BlitSurface(g_mox_vbb_surface1, &blit_rect, g_mox_vbb_surface2, &blit_rect);

    void* txtr;   // pointer to the locked pixels
    int   pitch;  // pitch of the locked pixels ... length of row in bytes


    SDL_DestroyTexture(g_mox_vbb_texture);

    sdl_screen_pixel_width  = mox_screen_pixel_width  * mox_scale;
    sdl_screen_pixel_height = mox_screen_pixel_height * mox_scale;

    sdl_window_center_x = ((sdl_window_rect.w - sdl_screen_pixel_width ) / 2);
    sdl_window_center_y = ((sdl_window_rect.h - sdl_screen_pixel_height) / 2);
    mox_texture_rect.x = sdl_window_center_x;
    mox_texture_rect.y = sdl_window_center_y;
    mox_texture_rect.w = sdl_screen_pixel_width;
    mox_texture_rect.h = sdl_screen_pixel_height;

    g_mox_vbb_texture = SDL_CreateTexture(g_renderer, sdl_window_pixel_format, SDL_TEXTUREACCESS_STREAMING, sdl_screen_pixel_width, sdl_screen_pixel_height);
    if (g_mox_vbb_texture == nullptr)
    {
        sprintf(OutputDebugBuffer, "SDL_CreateTexture(): %s\n", SDL_GetError());
        OutputDebugStringA(OutputDebugBuffer);
        __debugbreak();
    }

    if (SDL_LockTexture(g_mox_vbb_texture, NULL, &txtr, &pitch) != 0)
    {
        sprintf(OutputDebugBuffer, "SDL_LockTexture(): %s\n", SDL_GetError());
        OutputDebugStringA(OutputDebugBuffer);
        __debugbreak();
    }

    // memcpy(txtr, g_mox_vbb_surface2->pixels, mox_screen_pixel_width * mox_screen_pixel_height * sizeof(SDL_Color));
    int* tsrc = (int*)g_mox_vbb_surface2->pixels;
    int* tdst = (int*)txtr;
    int itr_w;
    int itr_h;
    // for (itr_w = 0; itr_w < mox_screen_pixel_width; itr_w++)
    // {
    //     for (itr_h = 0; itr_h < mox_screen_pixel_height; itr_h++)
    //     {
    //         for (int y = 0; y < mox_scale; ++y)
    //         {
    //             for (int x = 0; x < mox_scale; ++x)
    //             {
    //                 *(tdst + (((itr_h * mox_scale) * sdl_screen_pixel_width) + (itr_w * mox_scale)) + ((y * pitch) + x)) = *(tsrc + ((itr_h * mox_screen_pixel_width) + itr_w));
    //             }
    //             // tdst += (pitch / sizeof(SDL_Color));
    //         }
    //     }
    // }
    int src_x;
    int src_y;
    int dst_x;
    int dst_y;
    for (itr_w = 0; itr_w < sdl_screen_pixel_width; itr_w++)
    {
        dst_x = itr_w;
        src_x = (itr_w / mox_scale);

        for (itr_h = 0; itr_h < sdl_screen_pixel_height; itr_h++)
        {
            dst_y = itr_h;
            src_y = (itr_h / mox_scale);

            *(tdst + (dst_y * sdl_screen_pixel_width) + dst_x) = *(tsrc + (src_y * mox_screen_pixel_width) + src_x);

        }
    }

    SDL_UnlockTexture(g_mox_vbb_texture);

}

// STU_SDL_main
int main(int, char**)
{



    Init_MoX();



    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
#pragma warning(suppress : 26812)  // https://learn.microsoft.com/en-us/cpp/code-quality/c26812?view=msvc-170
    SDL_WindowFlags sdl_window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    g_window = SDL_CreateWindow(sdl_window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, sdl_window_width, sdl_window_height, sdl_window_flags);
    if (g_window == nullptr)
    {
        sprintf(OutputDebugBuffer, "SDL_CreateWindow(): %s\n", SDL_GetError());
        OutputDebugStringA(OutputDebugBuffer);
        __debugbreak();
    }
    SDL_GetWindowPosition(g_window, &sdl_window_rect.x, &sdl_window_rect.y);
    SDL_GetWindowSize(g_window, &sdl_window_rect.w, &sdl_window_rect.h);

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (g_renderer == nullptr)
    {
        sprintf(OutputDebugBuffer, "SDL_CreateRenderer(): %s\n", SDL_GetError());
        OutputDebugStringA(OutputDebugBuffer);
        __debugbreak();
    }
    //SDL_RendererInfo info;
    //SDL_GetRendererInfo(renderer, &info);
    //SDL_Log("Current SDL_Renderer: %s", info.name);



    
    g_mox_vbb_surface1 = SDL_CreateRGBSurface(0, mox_screen_pixel_width, mox_screen_pixel_height, 8, 0, 0, 0, 0);

    unsigned int rmask, gmask, bmask, amask;
    int bpp;

    sdl_window_pixel_format = SDL_GetWindowPixelFormat(g_window);

    SDL_PixelFormatEnumToMasks(sdl_window_pixel_format, &bpp, &rmask, &gmask, &bmask, &amask);

    g_mox_vbb_surface2 = SDL_CreateRGBSurface(0, mox_screen_pixel_width, mox_screen_pixel_height, bpp, rmask, gmask, bmask, amask);

    g_mox_vbb_texture = SDL_CreateTexture(g_renderer, sdl_window_pixel_format, SDL_TEXTUREACCESS_STREAMING, mox_screen_pixel_width, mox_screen_pixel_height);
    if (g_mox_vbb_texture == nullptr)
    {
        sprintf(OutputDebugBuffer, "SDL_CreateTexture(): %s\n", SDL_GetError());
        OutputDebugStringA(OutputDebugBuffer);
        __debugbreak();
    }



    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& imio = ImGui::GetIO();
    imio.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    imio.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // void ImGui::StyleColorsDark(ImGuiStyle * dst)
    // ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    // ImVec4* colors = style->Colors;
    // colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);


    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(g_window, g_renderer);
    ImGui_ImplSDLRenderer2_Init(g_renderer);



    

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);



    // Main loop
    bool done = false;
    while (!done)
    {
        game_loop_count++;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(g_window))
                done = true;

            switch (event.type)
            {
                // if( e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP )
                case SDL_MOUSEMOTION:
                {
                    mouse_button_state = SDL_GetMouseState(&mouse_window_x, &mouse_window_y);
                    // mouse_button_left   = (mouse_button_state & SDL_BUTTON(SDL_BUTTON_LEFT));
                    // mouse_button_middle = (mouse_button_state & SDL_BUTTON(SDL_BUTTON_MIDDLE));
                    // mouse_button_right  = (mouse_button_state & SDL_BUTTON(SDL_BUTTON_RIGHT));
                    if (
                        (mouse_window_x >= mox_texture_rect.x) &&
                        (mouse_window_x <= (mox_texture_rect.x + (mox_texture_rect.w - 1))) &&
                        (mouse_window_y >= mox_texture_rect.y) &&
                        (mouse_window_y <= (mox_texture_rect.y + (mox_texture_rect.h - 1)))
                    )
                    {
                        mouse_in_mox_screen = true;
                        mox_mouse_position_x = (event.motion.x - mox_texture_rect.x);
                        mox_mouse_position_y = (event.motion.y - mox_texture_rect.y);
                    }
                    else
                    {
                        mouse_in_mox_screen = false;
                        mox_mouse_position_x = -1;
                        mox_mouse_position_y = -1;
                    }
                    
                    if (lock_mouse_in_mox_screen == true)
                    {
                        if(
                            (event.motion.x < mox_texture_rect.x) ||
                            (event.motion.x > (mox_texture_rect.x + (mox_texture_rect.w - 1))) ||
                            (event.motion.y < mox_texture_rect.y) ||
                            (event.motion.y > (mox_texture_rect.y + (mox_texture_rect.h - 1)))
                        )
                        {
                            if (event.motion.x < mox_texture_rect.x)
                            {
                                mouse_window_x = mox_texture_rect.x;
                            }
                            if (event.motion.x > (mox_texture_rect.x + (mox_texture_rect.w - 1)))
                            {
                                mouse_window_x = (mox_texture_rect.x + (mox_texture_rect.w - 1));
                            }
                            if (event.motion.y < mox_texture_rect.y)
                            {
                                mouse_window_y = mox_texture_rect.y;
                            }
                            if (event.motion.y > (mox_texture_rect.y + (mox_texture_rect.h - 1)))
                            {
                                mouse_window_y = (mox_texture_rect.y + (mox_texture_rect.h - 1));
                            }

                            SDL_WarpMouseInWindow(NULL, mouse_window_x, mouse_window_y);
                        }
                    }

                    if (lock_mouse_in_mox_screen == true)
                    {
                        mox_mouse_position_x = ((mox_mouse_position_x <   0) ?   0 : mox_mouse_position_x);
                        mox_mouse_position_x = ((mox_mouse_position_x > ((mox_screen_pixel_width  * mox_scale) - 1)) ? ((mox_screen_pixel_width  * mox_scale) - 1) : mox_mouse_position_x);
                        mox_mouse_position_y = ((mox_mouse_position_y <   0) ?   0 : mox_mouse_position_y);
                        mox_mouse_position_y = ((mox_mouse_position_y > ((mox_screen_pixel_height * mox_scale) - 1)) ? ((mox_screen_pixel_height * mox_scale) - 1) : mox_mouse_position_y);

                        User_Mouse_Handler(0b00000000, (mox_mouse_position_x / mox_scale), (mox_mouse_position_y / mox_scale));
                    }

                } break;

                // if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
                // justPressed = true;
                // else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
                // justReleased = true;
                case SDL_MOUSEBUTTONDOWN:
                {
                    // OR  if was 1 or is 1 then 1
                    // mouse_button_left = ((mouse_button_left == 1) || (event.button.button == SDL_BUTTON_LEFT));
                    if ((mouse_button_left   == 1) || (event.button.button == SDL_BUTTON_LEFT)   ) { mouse_button_left   = 1; }
                    if ((mouse_button_middle == 1) || (event.button.button == SDL_BUTTON_MIDDLE) ) { mouse_button_middle = 1; }
                    if ((mouse_button_right  == 1) || (event.button.button == SDL_BUTTON_RIGHT)  ) { mouse_button_right  = 1; }
                    if ((mouse_button_x1     == 1) || (event.button.button == SDL_BUTTON_X1)     ) { mouse_button_x1     = 1; }
                    if ((mouse_button_x2     == 1) || (event.button.button == SDL_BUTTON_X2)     ) { mouse_button_x2     = 1; }
                    if (
                        (event.button.button == SDL_BUTTON_LEFT) &&
                        (mouse_in_mox_screen == true)
                        )
                    {
                        lock_mouse_in_mox_screen = true;
                    }
                } break;

                case SDL_MOUSEBUTTONUP:
                {
                    // NAND  if was 1 and is 1 then 0
                    // mouse_button_left = !((mouse_button_left == 1) && (event.button.button == SDL_BUTTON_LEFT));
                    if ( (mouse_button_left   == 1) && (event.button.button == SDL_BUTTON_LEFT)   ) { mouse_button_left   = 0; }
                    if ( (mouse_button_middle == 1) && (event.button.button == SDL_BUTTON_MIDDLE) ) { mouse_button_middle = 0; }
                    if ( (mouse_button_right  == 1) && (event.button.button == SDL_BUTTON_RIGHT)  ) { mouse_button_right  = 0; }
                    if ( (mouse_button_x1     == 1) && (event.button.button == SDL_BUTTON_X1)     ) { mouse_button_x1     = 0; }
                    if ( (mouse_button_x2     == 1) && (event.button.button == SDL_BUTTON_X2)     ) { mouse_button_x2     = 0; }
                } break;

                // https://wiki.libsdl.org/SDL2/SDL_Keycode
                // Key Name                         SDL_Scancode Value  SDL_Keycode Value
                // "Pause" (the Pause / Break key)  SDL_SCANCODE_PAUSE  SDLK_PAUSE

                case SDL_KEYDOWN:
                {
                    // Pause / Unpause  Yay-Pause/Nay-Pause
                    if (event.key.keysym.sym == SDLK_PAUSE)
                    {
                        // game_pause = !game_pause;
                        game_pause = true;
                        break;
                    }

                    if (
                        (event.key.keysym.sym == SDLK_PAUSE) &&
                        ((event.key.keysym.mod & KMOD_CTRL) != 0)
                    )
                    {
                        game_pause = false;
                        break;
                    }

                    if(
                        (event.key.keysym.sym == SDLK_INSERT) &&
                        (mouse_in_mox_screen == true)
                    )
                    {
                        lock_mouse_in_mox_screen = false;
                    }
                    // key_pressed = ST_TRUE;
                    // scan_code_char_code = SDLVK_to_SCCS[VKCode];
                } break;

                case SDL_KEYUP:
                {
                } break;

                case SDL_WINDOWEVENT:
                {
                    switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_MOVED:
                        {
                            SDL_GetWindowPosition(g_window, &sdl_window_rect.x, &sdl_window_rect.y);
                            SDL_GetWindowSize(g_window, &sdl_window_rect.w, &sdl_window_rect.h);
                        } break;
                        // SDL_WINDOWEVENT_RESIZED
                        // window has been resized to data1 x data2; this event is always preceded by SDL_WINDOWEVENT_SIZE_CHANGED
                        // SDL_WINDOWEVENT_SIZE_CHANGED
                        // window size has changed, either as a result of an API call or through the system or user changing the window size; this event is followed by SDL_WINDOWEVENT_RESIZED if the size was changed by an external event, i.e.the user or the window manager

                        // SDL_WINDOW_MOVED nor SDL_WINDOW_RESIZED
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            // SDL_Window* window = SDL_GetWindowFromID(event->window.windowID);
                            // SDL_Renderer* renderer = SDL_GetRenderer(window);
                            // Set_Offscreen_Buffer(event->window.data1, event->window.data2);
                            sdl_window_rect.w = event.window.data1;
                            sdl_window_rect.h = event.window.data2;
                            // SDL_GetWindowPosition(g_window, &sdl_window_rect.x, &sdl_window_rect.y);
                            // SDL_GetWindowSize(g_window, &sdl_window_rect.w, &sdl_window_rect.h);
                        } break;

                        case SDL_WINDOWEVENT_FOCUS_GAINED:
                        {
                            // printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
                        } break;

                        case SDL_WINDOWEVENT_EXPOSED:
                        {
                            // SDL_Window* window = SDL_GetWindowFromID(event->window.windowID);
                            // SDL_Renderer* renderer = SDL_GetRenderer(window);
                            // Update_Display();
                        } break;
                    }
                } break;
            }

        }



        video_frame_count++;

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();


        ImGui_Windows();


        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(g_renderer, imio.DisplayFramebufferScale.x, imio.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(g_renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(g_renderer);



        Update_MoX_Texture();
        SDL_RenderCopy(g_renderer, g_mox_vbb_texture, NULL, &mox_texture_rect);



        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(g_renderer);
    }



    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0;
}





void ImGui_Windows(void)
{

    ImGuiIO& imio = ImGui::GetIO();



    // ImGuiStyle& style = ImGui::GetStyle();
    // ImVec4* colors = style.Colors;
    // colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.06f, 0.235f));
    if (mouse_in_mox_screen == true)
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.06f, 0.235f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.06f, 0.06f, 0.06f, 0.235f));
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.06f, 0.06f, 0.06f, 0.235f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.06f, 0.06f, 0.06f, 0.5f));
    }

    // ImGuiCond_FirstUseEver   window does not yet exist in imgui.ini
    // ImGuiCond_Once           once per session
    // ImGui::SetNextWindowPos(ImVec2(200, 200), ImGuiCond_FirstUseEver);
    // ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(260, 130), ImGuiCond_Once);
    ImGui::Begin("ImGui Windows");
    ImGui::Checkbox("Options", &show_options_window);
    ImGui::Checkbox("Frame-Rate", &show_framerate_window);
    ImGui::Checkbox("Inputs", &show_inputs_window);
    ImGui::Checkbox("Demo Window", &show_demo_window);
    ImGui::End();

    // ImGui::SetNextWindowPos(ImVec2(240, 0), ImGuiCond_Once);
    // ImGui::SetNextWindowSize(ImVec2(170, 135), ImGuiCond_Once);
    ImGui::Begin("Coordinates");
    ImGui::Text("Application Window");
    ImGui::Text("x,y: (%d, %d)", sdl_window_rect.x, sdl_window_rect.y);
    ImGui::Text("w,h: (%d, %d)", sdl_window_rect.w, sdl_window_rect.h);
    ImGui::Text("MoX Texture");
    ImGui::Text("x,y: (%d, %d)", mox_texture_rect.x, mox_texture_rect.y);
    ImGui::Text("w,h: (%d, %d)", mox_texture_rect.w, mox_texture_rect.h);
    ImGui::Text("MoX Mouse");
    if (mouse_in_mox_screen)
        ImGui::Text("position: (%d, %d)", mox_mouse_position_x, mox_mouse_position_y);
    else
        ImGui::Text("position: <INVALID>");
    ImGui::Text("Locked: ");
    ImGui::SameLine();
    if (lock_mouse_in_mox_screen)
        ImGui::Text("TRUE");
    else
        ImGui::Text("FALSE");
    ImGui::End();

    ImGui::Begin("Mouse");
    ImGui::Text("Coordinates");
    ImGui::Text("x: %d", mouse_window_x);
    ImGui::Text("y: %d", mouse_window_y);
    ImGui::Text("Buttons");
    ImGui::Text("left: %d", mouse_button_left);
    ImGui::Text("middle: %d", mouse_button_middle);
    ImGui::Text("right: %d", mouse_button_right);
    ImGui::Text("x1: %d", mouse_button_x1);
    ImGui::Text("x2: %d", mouse_button_x2);
    ImGui::End();

    // framerate 260,50

    if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }


    if (show_options_window)
    {
        ImGui::Begin("Options");
        ImGui::Text("Scale:");
        ImGui::RadioButton("1x", &mox_scale, 1); ImGui::SameLine();
        ImGui::RadioButton("2x", &mox_scale, 2); ImGui::SameLine();
        ImGui::RadioButton("3x", &mox_scale, 3); ImGui::SameLine();
        ImGui::RadioButton("4x", &mox_scale, 4);
        ImGui::End();
    }

    if (show_framerate_window)
    {
        ImGui::Begin("Frame-Rate");
        ImGui::Text("average %.3f ms/frame (%.1f FPS)", 1000.0f / imio.Framerate, imio.Framerate);
        ImGui::Text("Game Loop: %d", game_loop_count);
        ImGui::Text("Video Frame: %d", video_frame_count);
        ImGui::Text("Game Pause: %d", game_pause);
        ImGui::End();
    }

    if (show_inputs_window)
    {
        ImGui::Begin("Inputs");
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse pos: (%g, %g)", imio.MousePos.x, imio.MousePos.y);
        else
            ImGui::Text("Mouse pos: <INVALID>");
        ImGui::Text("Mouse delta: (%g, %g)", imio.MouseDelta.x, imio.MouseDelta.y);
        ImGui::Text("Mouse down:");
        for (int i = 0; i < IM_ARRAYSIZE(imio.MouseDown); i++) if (ImGui::IsMouseDown(i)) { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, imio.MouseDownDuration[i]); }
        ImGui::Text("Mouse wheel: %.1f", imio.MouseWheel);

        struct funcs { static bool IsLegacyNativeDupe(ImGuiKey) { return false; } };
        ImGuiKey start_key = ImGuiKey_NamedKey_BEGIN;
        ImGui::Text("Keys down:");         for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) continue; ImGui::SameLine(); ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key); }
        ImGui::Text("Keys mods: %s%s%s%s", imio.KeyCtrl ? "CTRL " : "", imio.KeyShift ? "SHIFT " : "", imio.KeyAlt ? "ALT " : "", imio.KeySuper ? "SUPER " : "");
        ImGui::Text("Chars queue:");       for (int i = 0; i < imio.InputQueueCharacters.Size; i++) { ImWchar c = imio.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
        ImGui::End();
    }

    if (mouse_in_mox_screen == true)
    {
        ImGui::PopStyleColor(4);
    }

}
