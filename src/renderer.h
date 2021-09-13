#ifndef RENDERER_H_
#define RENDERER_H_

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <stdint.h>

#define LCD_WIDTH       (128)
#define LCD_HEIGHT      (64)
#define BYTES_PER_LINE  (16)

typedef void(*render_function_t)(uint8_t* back_buffer);

inline uint16_t pixel_byte(int16_t x, int16_t y)
{
    return (BYTES_PER_LINE*y)+(x/8);
}

inline uint8_t pixel_bitmask(int16_t x)
{
    return (0x01 << (x%8));
}

inline void set_pixel(uint8_t* buffer, uint8_t x, uint8_t y)
{
    uint16_t byte = pixel_byte(x, y);
    uint8_t mask = pixel_bitmask(x);
    buffer[byte] |= mask;
}

inline void reset_pixel(uint8_t* buffer, uint8_t x, uint8_t y)
{
    uint16_t byte = pixel_byte(x, y);
    uint8_t mask = pixel_bitmask(x);
    buffer[byte] &= ~mask;
}

extern Adafruit_SH1107* display;

// Copy a single pixel
void copy_pixel(const uint8_t* src,
                uint8_t* dst,
                int16_t x_src,
                int16_t y_src,
                int16_t x_dst,
                int16_t y_dst);

// Copy a scan line
void copy_line(const uint8_t* src,
               uint8_t* dst,
               int16_t y_src,
               int16_t y_dst);

void copy_block(const uint8_t* src,
                uint8_t* dst,
                int16_t x_src,
                int16_t y_src,
                int16_t x_dst,
                int16_t y_dst,
                int16_t w,
                int16_t h);

void blit_buffer(uint8_t* buffer);

void render_init();
void push_render_function(render_function_t func);
void pop_render_function();
void render();

#endif // RENDERER_H_
