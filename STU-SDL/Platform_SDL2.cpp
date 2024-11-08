#include "Configuration.hpp"
#include "Platform.hpp"
#include "STU-SDL.hpp"

#include <SDL.h>
#include <SDL_image.h>

#include <string>  /* std::string */



// SDL_Window* g_window = NULL;
// SDL_Renderer* g_renderer = NULL;
// SDL_Texture* g_texture = NULL;

struct sdl2_offscreen_buffer
{
	void * memory;
	int width;
	int height;
	int pitch;
	int bytes_per_pixel;
};

struct sdl2_offscreen_buffer offscreen_buffer;



void Load_PNG_Texture(std::string path)
{
	SDL_Surface* surface = IMG_Load(path.c_str());
	g_texture = SDL_CreateTextureFromSurface(g_renderer, surface);
	SDL_FreeSurface(surface);
}


void Set_Offscreen_Buffer(int width, int height)
{
	size_t offscreen_buffer_memory_size;
	int bytes_per_pixel;

	if(offscreen_buffer.memory != NULL)
	{
		free(offscreen_buffer.memory);
	}

	if(g_texture != NULL)
	{
		SDL_DestroyTexture(g_texture);
	}

	g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

	offscreen_buffer.bytes_per_pixel = 4;
	offscreen_buffer.width = width;
	offscreen_buffer.height = height;

	offscreen_buffer_memory_size = ((offscreen_buffer.width * offscreen_buffer.height) * offscreen_buffer.bytes_per_pixel);

	offscreen_buffer.memory = malloc(offscreen_buffer_memory_size);

}


void Pump_Events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		if (event.type == SDL_QUIT)
		{
			quit_flag = 1;
		}
	}
}


void Update_Display()
{
	// ¿ SDL_RenderCopy() the final texture here ?
	
	// // Rendering
	// ImGui::Render();
	// SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	// SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
	// SDL_RenderClear(renderer);
	// ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
	// SDL_RenderPresent(renderer);

	// Clear screen
	SDL_RenderClear(g_renderer);
	// Render texture to screen
	SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
	// Update screen
	SDL_RenderPresent(g_renderer);

}


int Platform_Startup(void)
{

	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	/*
		¿¿¿
		LazyFoo
		// Set texture filtering to linear
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
		???
	*/

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// SDL2 uses a global screen space coordinate system
	// set the window creation coordinates for the display, realtive to the global screen space

	
	// Create window with SDL_Renderer graphics context
#pragma warning(suppress : 26812)  // https://learn.microsoft.com/en-us/cpp/code-quality/c26812?view=msvc-170
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	// SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	// g_window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_SHOWN);
	g_window = SDL_CreateWindow(sdl_window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, sdl_window_width, sdl_window_height, window_flags);
	if (g_window == nullptr)
	{
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}
	// SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	// g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
	g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (g_renderer == nullptr)
	{
		SDL_Log("Error creating SDL_Renderer!");
		return 0;
	}
	//SDL_RendererInfo info;
	//SDL_GetRendererInfo(renderer, &info);
	//SDL_Log("Current SDL_Renderer: %s", info.name);



	// Initialize renderer color
	SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	// Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	IMG_Init(imgFlags);

	// Set_Offscreen_Buffer(window_width, window_height);

}


int Platform_Shutdown(void)
{
	if (NULL != g_texture)
	{
		SDL_DestroyTexture(g_texture);
	}
	if (NULL != g_renderer)
	{
		SDL_DestroyRenderer(g_renderer);
	}
	if (NULL != g_window)
	{
		SDL_DestroyWindow(g_window);
	}
	IMG_Quit();
	SDL_Quit();
	// TODO  test for shutdown failure?
	return 0;
}



void Event_Handler(SDL_Event* event)
{

	switch (event->type)
	{

		case SDL_WINDOWEVENT:
		{
			switch (event->window.event)
			{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					// SDL_Window* window = SDL_GetWindowFromID(event->window.windowID);
					// SDL_Renderer* renderer = SDL_GetRenderer(window);
					Set_Offscreen_Buffer(event->window.data1, event->window.data2);
				} break;

				case SDL_WINDOWEVENT_FOCUS_GAINED:
				{
					// printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
				} break;

				case SDL_WINDOWEVENT_EXPOSED:
				{
					// SDL_Window* window = SDL_GetWindowFromID(event->window.windowID);
					// SDL_Renderer* renderer = SDL_GetRenderer(window);
					Update_Display();
				} break;
			}
		} break;
	}

}
