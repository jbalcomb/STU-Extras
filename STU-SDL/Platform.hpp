#pragma once

#include <string>  /* std::string */

#include <SDL.h>
#include <SDL_image.h>
extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;
extern SDL_Texture* g_texture;



extern char window_title[];
extern int window_width;
extern int window_height;

extern int quit_flag;

void Load_PNG_Texture(std::string path);

int Platform_Startup(void);
int Platform_Shutdown(void);

void Pump_Events(void);

void Update_Display(void);
