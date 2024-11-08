/*
    SDL_Window
    SDL_Renderer
    SDL_Texture
    SDL_Surface

    width, height
    pixel format

*/
#include <SDL.h>





// int SDL_QueryTexture(SDL_Texture* texture, Uint32* format, int* access, int* w, int* h);
// the raw format of the texture; the actual format may differ, but pixel transfers will use this format (one of the SDL_PixelFormatEnum values)
// the actual access to the texture (one of the SDL_TextureAccess values)
// the width of the texture in pixels
// the height of the texture in pixels
/*
SDL_TextureAccess
    An enumeration of texture access patterns.
Values
    SDL_TEXTUREACCESS_STATIC        changes rarely, not lockable
    SDL_TEXTUREACCESS_STREAMING     changes frequently, lockable
    SDL_TEXTUREACCESS_TARGET        can be used as a render target
*/

void Query_Texture(SDL_Texture* texture)
{
    Uint32 format;
    int access;
    int w;
    int h;

    SDL_QueryTexture(texture, &format, nullptr, &w, &h);

    // SDL_GetPixelFormatName()
    // SDL_MasksToPixelFormatEnum()
    // SDL_PixelFormatEnumToMasks()

}
