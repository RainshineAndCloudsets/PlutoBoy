#include "../../non_core/graphics_out.h"
#include "../../non_core/logger.h"

#include "SDL/SDL.h"


typedef int (*Int_2D )[GB_PIXELS_X];

static Uint32 colors[4]; // Store GameBoy monchrome colors
static SDL_Surface *screen;
static Int_2D screen_buffer;

static int screen_width;
static int screen_height;


/*  Initialise graphics and create win_x by win_y pixel
 *  screen. Keeps track of GB_PIXELS_Y by GB_PIXELS_X screen. 
 *  return  1 if successful, 0 otherwise */
int init_screen(int win_x, int win_y, int (*const pixels)[GB_PIXELS_X]) {
       
    screen_width = win_x;
    screen_height = win_y;

    screen_buffer = pixels;

    if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) {
        log_message(LOG_ERROR, "Could not initialize SDL: %s.\n", SDL_GetError());
        return 0;
    }

    SDL_WM_SetCaption("Gameboy","");
    screen = SDL_SetVideoMode(screen_width, screen_height, 32 ,SDL_DOUBLEBUF);

   // cols[0] = SDL_MapRGB(screen->format, 255, 255, 255); /* White */
   // cols[1] = SDL_MapRGB(screen->format, 170, 170, 170); /* Light Grey */
   // cols[2] = SDL_MapRGB(screen->format, 85, 85, 85); /* Dark Grey */
   // cols[3] = SDL_MapRGB(screen->format, 0, 0, 0); /* Black */

   // cols[0] = SDL_MapRGB(screen->format, 155, 187, 14);
   // cols[1] = SDL_MapRGB(screen->format, 115, 160, 103);
   // cols[2] = SDL_MapRGB(screen->format, 53, 98, 55);
   // cols[3] = SDL_MapRGB(screen->format, 15, 56, 14);

    colors[0] = SDL_MapRGB(screen->format, 255, 255, 255);
    colors[1] = SDL_MapRGB(screen->format, 136, 192, 112);
    colors[2] = SDL_MapRGB(screen->format, 48,  104, 80);
    colors[3] = SDL_MapRGB(screen->format, 8, 24, 32);

    return 1;

}


inline static void fill_rect(int x, int y, int w, int h, Uint32 color)
{
    
    SDL_Rect rect = {x, y, w, h};
    SDL_FillRect(screen, &rect, color);
}


/* Draw a GameBoy pixel of the given co-ordinates and of
 * the given 32bit color */
inline static void draw_pix(Uint32 color, int x, int y)
{
    int width_inc = screen_width/GB_PIXELS_X;
    int height_inc = screen_height/GB_PIXELS_Y;
    fill_rect(x*width_inc,y*height_inc,width_inc,height_inc, color);
        
}


/*  Update the screen output */
void draw_screen() {
    for (int y = 0; y < GB_PIXELS_Y; y++) {
        for (int x = 0; x < GB_PIXELS_X; x++) {
            draw_pix(colors[screen_buffer[y][x]], x, y);
        }
    } 
    
    SDL_Flip(screen);   
}



