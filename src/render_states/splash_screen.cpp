#include "splash_screen.h"

#include "images.h"
#include "renderer.h"
#include "utility.h"

#include <algorithm>
#include <deque>

#define FADE_RATE_MS 1
#define FADE_HOLD_MS 2500
#define FADE_CHUNK_SIZE 64

typedef enum {
    CLEAR = 0,
    FADE_IN,
    HOLD,
    FADE_OUT,
    COMPLETE
} splash_state_t;

typedef struct
{
    uint8_t x;
    uint8_t y;
} pixel_index_t;

void splash_screen_render(uint8_t* buffer)
{
    static uint32_t timer;
    static splash_state_t state = CLEAR;
    static std::deque<pixel_index_t> index_deque;
    static std::deque<pixel_index_t>::iterator pixel_idx;
    switch(state)
    {
        case CLEAR:
            timer = millis();
            index_deque.resize(LCD_WIDTH * LCD_HEIGHT);
            // Clear the back buffer
            display->clearDisplay();
            for(uint8_t y = 0; y < LCD_HEIGHT; ++y)
            {
                for(uint8_t x = 0; x < LCD_WIDTH; ++x)
                {
                    index_deque[(y * LCD_WIDTH) + x].x = x;
                    index_deque[(y * LCD_WIDTH) + x].y = y;
                }
            }
            std::random_shuffle(index_deque.begin(), index_deque.end());
            pixel_idx = index_deque.begin();
            state = FADE_IN;
            break;
        case FADE_IN:
            if ((millis() - timer) > FADE_RATE_MS)
            {
                for(uint8_t i = 0; i < FADE_CHUNK_SIZE; ++i)
                {
                    uint8_t image_byte = pgm_read_byte(&(DI_FULL.data[DATA_COORDINATE(pixel_idx->x, pixel_idx->y)]));
                    if (((image_byte << (pixel_idx->x % 8) & 0x80)))
                    {
                        display->drawPixel(pixel_idx->x, pixel_idx->y, MONOOLED_WHITE);
                        timer = millis();
                    }

                    ++pixel_idx;
                    if (index_deque.end() == pixel_idx)
                    {
                        display->setTextColor(SH110X_WHITE);
                        display->setTextSize(1);
                        display->setCursor(12, 56);
                        display->print("Digital Industries");
                        pixel_idx = index_deque.begin();
                        state = HOLD;
                        break;
                    }
                }
            }
            break;
        case HOLD:
            if ((millis() - timer) > FADE_HOLD_MS)
            {
                timer = millis();
                state = FADE_OUT;
            }
            break;
        case FADE_OUT:
            if ((millis() - timer) > FADE_RATE_MS)
            {
                for(uint8_t i = 0; i < FADE_CHUNK_SIZE; ++i)
                {
                    display->drawPixel(pixel_idx->x, pixel_idx->y, MONOOLED_BLACK);
                    timer = millis();

                    ++pixel_idx;
                    if (index_deque.end() == pixel_idx)
                    {
                        pixel_idx = index_deque.begin();
                        state = COMPLETE;
                        break;
                    }
                }
            }
            break;
        case COMPLETE:
            pop_render_function();
            break;
        default:
            break;
    }
}
