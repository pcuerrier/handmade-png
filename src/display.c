#include "display.h"
#include "swap.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*******************************************************************************
 * Initialize SDL Window & Renderer
*******************************************************************************/
SDL_API initialize_window(void)
{
    SDL_API result =
    {
        .window = NULL,
        .renderer = NULL,
        .fullscreen = 0,
        .is_running = 0
    };

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        // Error when initiliazing SDL
        fprintf(stderr, "Error when initializing SDL : %s\n", SDL_GetError());
        return result;
    }

    result.window = SDL_CreateWindow("CPU-Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE);
    if (!result.window)
    {
        fprintf(stderr, "Error when creating the window : %s\n", SDL_GetError());
        return result;
    }
    result.fullscreen = 0;

    result.renderer = SDL_CreateRenderer(result.window, 0, 0);
    if (!result.renderer)
    {
        fprintf(stderr, "Error when creating the renderer : %s\n", SDL_GetError());
        return result;
    }

    result.is_running = 1;
    return result;
}

/*******************************************************************************
 * Destroy SDL Window & Renderer
*******************************************************************************/
void destroy_window(SDL_API* sdl)
{
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
}

/*******************************************************************************
 * Clear Color Buffer
*******************************************************************************/
void clear_color_buffer(ColorBuffer* color_buffer, uint32_t color)
{
    // Set every pixel to 'color'
    for (uint32_t row = 0; row < color_buffer->height; ++row)
    {
        for (uint32_t column = 0; column < color_buffer->width; ++column)
        {
            color_buffer->memory[color_buffer->width * row + column] = color;
        }
    }
}

/*******************************************************************************
 * Create Color Buffer
*******************************************************************************/
void create_color_buffer(SDL_API* sdl, ColorBuffer* color_buffer)
{
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(sdl->window, &width, &height);
    color_buffer->memory = (uint32_t*)malloc(sizeof(uint32_t) * width * height);
    color_buffer->texture = SDL_CreateTexture(
        sdl->renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );
    color_buffer->width = width;
    color_buffer->height = height;
}

/*******************************************************************************
 * Resize Color Buffer
*******************************************************************************/
void resize_color_buffer(SDL_API* sdl, ColorBuffer* color_buffer, uint32_t width, uint32_t height)
{
    if (color_buffer->texture)
    {
        SDL_DestroyTexture(color_buffer->texture);
    }
    if (color_buffer->memory)
    {
        free(color_buffer->memory);
    }
    color_buffer->memory = (uint32_t*)malloc(sizeof(uint32_t) * width * height);
    color_buffer->texture = SDL_CreateTexture(
        sdl->renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );
    color_buffer->width = width;
    color_buffer->height = height;
}

/*******************************************************************************
 * Destroy Color Buffer
*******************************************************************************/
void destroy_color_buffer(ColorBuffer* color_buffer)
{
    color_buffer->width = 0;
    color_buffer->height = 0;
    SDL_DestroyTexture(color_buffer->texture);
    free(color_buffer->memory);
    color_buffer->memory = NULL;
}

/*******************************************************************************
 * Draw a single pixel
*******************************************************************************/
void draw_pixel(ColorBuffer* color_buffer, int x, int y, uint32_t color)
{
    if (x >= 0 && x < (int)color_buffer->width &&
        y >= 0 && y < (int)color_buffer->height)
    {
        color_buffer->memory[color_buffer->width * y + x] = color;
    }
}

/*******************************************************************************
 * Draw Grid
*******************************************************************************/
void draw_grid(ColorBuffer* color_buffer, uint32_t size, uint32_t color)
{
    if (size > 0)
    {
        for (uint32_t row = 0; row < color_buffer->height; row += size)
        {
            for (uint32_t col = 0; col < color_buffer->width; col += size)
            {
                color_buffer->memory[color_buffer->width * row + col] = color;
            }
        }
    }
}

/*******************************************************************************
 * Draw Rectangle
*******************************************************************************/
void draw_rect(ColorBuffer* color_buffer, int x, int y, uint32_t width,
               uint32_t height, uint32_t color)
{
    for (int row = 0; row < (int)height; ++row)
    {
        for (int column = 0; column < (int)width; ++column)
        {
            int current_x = x + column;
            int current_y = y + row;
            draw_pixel(color_buffer, current_x, current_y, color);
        }
    }
}

/*******************************************************************************
 * Draw Line
*******************************************************************************/
void draw_line(ColorBuffer* color_buffer, int x0, int y0, int x1, int y1, uint32_t color)
{
    // Based on DDA
    int delta_x = x1 - x0;
    int delta_y = y1 - y0;

    int side_length = abs(delta_x) >= abs(delta_y) ?
                      abs(delta_x) :
                      abs(delta_y);

    // How much incerement x and y each step
    float x_inc = delta_x / (float)side_length;
    float y_inc = delta_y / (float)side_length;

    float current_x = (float)x0;
    float current_y = (float)y0;

    for (int i = 0; i <= side_length; ++i)
    {
        draw_pixel(color_buffer, (int)round(current_x),
                   (int)round(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}

void draw_triangle(ColorBuffer* color_buffer, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    draw_line(color_buffer, x0, y0, x1, y1, color);
    draw_line(color_buffer, x1, y1, x2, y2, color);
    draw_line(color_buffer, x2, y2, x0, y0, color);
}

/*void draw_texel(ColorBuffer* color_buffer, int x, int y, uint32_t* texture, vec4_t point_a, vec4_t point_b, vec4_t point_c, tex2_t a_uv, tex2_t b_uv, tex2_t c_uv)
{
    vec2_t p = { x, y };
    vec2_t a = { point_a.x, point_a.y };
    vec2_t b = { point_b.x, point_b.y };
    vec2_t c = { point_c.x, point_c.y };

    // Calculate the barycentric coordinates of our point 'p' inside the triangle
    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // Variables to store the interpolated values of U, V, and also 1/w for the current pixel
    float interpolated_u;
    float interpolated_v;
    float interpolated_reciprocal_w;

    // Perform the interpolation of all U/w and V/w values using barycentric weights and a factor of 1/w
    interpolated_u = (a_uv.u / point_a.w) * alpha + (b_uv.u / point_b.w) * beta + (c_uv.u / point_c.w) * gamma;
    interpolated_v = (a_uv.v / point_a.w) * alpha + (b_uv.v / point_b.w) * beta + (c_uv.v / point_c.w) * gamma;

    // Also interpolate the value of 1/w for the current pixel
    interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

    // Now we can divide back both interpolated values by 1/w
    interpolated_u /= interpolated_reciprocal_w;
    interpolated_v /= interpolated_reciprocal_w;

    // Map the UV coordinate to the full texture width and height
    int tex_x = abs((int)(interpolated_u * texture_width)) % texture_width;
    int tex_y = abs((int)(interpolated_v * texture_height)) % texture_height;

    interpolated_reciprocal_w = 1.0f - interpolated_reciprocal_w;

    // Only draw the pixel if the depth value is less than the one previously stored in the z-buffer
    if (interpolated_reciprocal_w < z_buffer[(color_buffer->width * y) + x])
    {
        draw_pixel(color_buffer, x, y, texture[(texture_width * tex_y) + tex_x]);
        // Update the z-buffer with the 1/w
        z_buffer[(color_buffer->width * y) + x] = interpolated_reciprocal_w;
    }
}*/

/*void draw_textured_triangle(ColorBuffer* color_buffer,
                            int x0, int y0, float z0, float w0,
                            int x1, int y1, float z1, float w1,
                            int x2, int y2, float z2, float w2,
                            float u0, float v0,
                            float u1, float v1,
                            float u2, float v2,
                            uint32_t* texture)
{
    // Sort vertices by ascending y-coordinate (y0 < y1 < y2)
    if (y0  > y1)
    {
        int_swap(x0, x1);
        int_swap(y0, y1);
        float_swap(z0, z1);
        float_swap(w0, w1);
        float_swap(u0, u1);
        float_swap(v0, v1);
    }
    if (y1  > y2)
    {
        int_swap(x1, x2);
        int_swap(y1, y2);
        float_swap(z1, z2);
        float_swap(w1, w2);
        float_swap(u1, u2);
        float_swap(v1, v2);
    }
    // y0 y1 might have changed due to swap
    if (y0  > y1)
    {
        int_swap(x0, x1);
        int_swap(y0, y1);
        float_swap(z0, z1);
        float_swap(w0, w1);
        float_swap(u0, u1);
        float_swap(v0, v1);
    }

    vec4_t point_a = { x0, y0, z0, w0 };
    vec4_t point_b = { x1, y1, z1, w1 };
    vec4_t point_c = { x2, y2, z2, w2 };
    tex2_t a_uv = { u0, v0 };
    tex2_t b_uv = { u1, v1 };
    tex2_t c_uv = { u2, v2 };

    ///////////////////////////////////////////////////////
    // Render the upper part of the triangle (flat-bottom)
    ///////////////////////////////////////////////////////
    float inv_slope_1 = 0.0f;
    float inv_slope_2 = 0.0f;

    if (y1 - y0 != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y1 - y0 != 0.0f)
    {
        for (int y = y0; y <= y1; ++y)
        {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start)
            {
                int_swap(x_start, x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; ++x)
            {
                // Draw our pixel with the color that comes from the texture
                draw_texel(color_buffer, x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }

    ///////////////////////////////////////////////////////
    // Render the bottom part of the triangle (flat-top)
    ///////////////////////////////////////////////////////
    inv_slope_1 = 0.0f;
    inv_slope_2 = 0.0f;

    if (y2 - y1 != 0.0f) inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
    if (y2 - y0 != 0.0f) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y2 - y1 != 0.0f)
    {
        for (int y = y1; y <= y2; ++y)
        {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start)
            {
                int_swap(x_start, x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; ++x)
            {
                // Draw our pixel with the color that comes from the texture
                draw_texel(color_buffer, x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }
}*/