#include "display.h"
#include <stdio.h>
#include <SDL/SDL_main.h>

/*******************************************************************************
 * Constants
*******************************************************************************/
const int FPS = 60;
const float FRAME_TARGET_TIME = 16.66666666666667f;
const float SCALE = 640.0f;

/*******************************************************************************
 * Functions
*******************************************************************************/
void setup()
{
}

void process_input(SDL_API* sdl, ColorBuffer* color_buffer)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            {
                sdl->is_running = 0;
            } break;

            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    sdl->is_running = 0;
                }
                else if (event.key.keysym.sym == SDLK_f)
                {
                    sdl->fullscreen = !sdl->fullscreen;
                    SDL_SetWindowFullscreen(sdl->window, sdl->fullscreen);
                }
            } break;

            case SDL_WINDOWEVENT_RESIZED:
            {
                int width = event.window.data1;
                int height = event.window.data2;
                resize_color_buffer(sdl, color_buffer, width, height);
            } break;
        }
    }
}

void update(const SDL_API* sdl, uint32_t window_width, uint32_t window_height)
{
    
}

void render(const SDL_API* sdl, ColorBuffer* color_buffer)
{
    // AA RR GG BB
    SDL_UpdateTexture(
        color_buffer->texture,
        NULL,
        color_buffer->memory,
        sizeof(uint32_t) * color_buffer->width
    );
    SDL_RenderCopy(sdl->renderer, color_buffer->texture, NULL, NULL);
    clear_color_buffer(color_buffer, 0xFF18191A);

    SDL_RenderPresent(sdl->renderer);
}

int main(int argc, char** argv)
{
    SDL_API sdl = initialize_window();
    if (!sdl.window || !sdl.renderer)
    {
        return 1;
    }

    ColorBuffer color_buffer =
    {
        .memory = NULL,
        .texture = NULL,
        .width = 0,
        .height = 0
    };
    create_color_buffer(&sdl, &color_buffer);
    clear_color_buffer(&color_buffer, 0xFF18191A);

    setup();
    
    // Main Loop
    while (sdl.is_running)
    {
        uint32_t start_frame_ticks = (uint32_t)SDL_GetTicks();
        process_input(&sdl, &color_buffer);
        update(&sdl, color_buffer.width, color_buffer.height);
        render(&sdl, &color_buffer);
        uint32_t elapsed_frame_ticks = (uint32_t)SDL_GetTicks() - start_frame_ticks;
        if ((float)elapsed_frame_ticks <= FRAME_TARGET_TIME)
        {
            SDL_Delay((uint32_t)(FRAME_TARGET_TIME - (float)elapsed_frame_ticks));
        }
    }

    // Free resources
    destroy_color_buffer(&color_buffer);
    destroy_window(&sdl);
    SDL_Quit();

    return 0;
}