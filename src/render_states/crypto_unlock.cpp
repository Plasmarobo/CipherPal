#include "crypto_unlock.h"

#include "buttons.h"
#include "renderer.h"
#include "utility.h"

#include <algorithm>
#include <vector>
#include <set>

#define KEY_ROTATE_MS 5000
#define CYCLE_RATE_MS 1000
#define CYCLE_COUNT 8
#define CELL_STATE_LOCKED 0x01
#define CHARACTERS_PER_LINE 11
#define UP_KEY 0
#define DOWN_KEY 2
#define SEL_KEY 1
#define KEY_COUNT 3

static const uint8_t ROWS = 3;
static const uint8_t COLUMNS = CHARACTERS_PER_LINE;
static const uint16_t CELLS = ROWS * COLUMNS;

typedef struct
{
    uint8_t codepoint;
    uint8_t state;
} crypto_index_t;

typedef enum
{
    CRYPTO_UNLOCK_ENTER,
    CRYPTO_UNLOCK_STEP,
    CRYPTO_UNLOCK_SUCCESS,
    CRYPTO_UNLOCK_EXIT,
    CRYPTO_UNLOCK_MAX
} crypto_unlock_state_t;

static crypto_index_t cell[CELLS];
static crypto_unlock_state_t state;
static uint32_t key_timer;
static uint32_t cycle_timer;
static uint8_t key_codepoints[KEY_COUNT];
static const uint8_t key_icons[KEY_COUNT] = {
    0x18, // Up Arrow
    0x1A, // Right Arrow
    0x19, // Down Arrow
};
static const uint8_t key_separator = 0x3A;
static uint8_t buttons;
static std::vector<uint8_t> codepoint_set;

void draw_cells()
{
    for(uint16_t i = 0; i < CELLS; ++i)
    {
        uint8_t x = 5 + ((i % CHARACTERS_PER_LINE) * 11);
        uint8_t y = 4 + ((i / CHARACTERS_PER_LINE) * 17);
        display->drawChar(x,
                          y,
                          (char)cell[i].codepoint,
                          (cell[i].state & CELL_STATE_LOCKED) ? MONOOLED_BLACK : MONOOLED_WHITE,
                          (cell[i].state & CELL_STATE_LOCKED) ? MONOOLED_WHITE : MONOOLED_BLACK,
                          2);
    }
}

void draw_keys()
{
    uint8_t y = 57;
    uint8_t x = 18;
    for (uint8_t i = 0; i < KEY_COUNT; ++i)
    {
        display->drawChar(x, y, key_icons[i], MONOOLED_WHITE, MONOOLED_BLACK, 1);
        x += 7;
        display->drawChar(x, y, key_separator, MONOOLED_WHITE, MONOOLED_BLACK, 1);
        x += 7;
        display->drawChar(x, y, key_codepoints[i], MONOOLED_WHITE, MONOOLED_BLACK, 1);
        x += 23;
    }
}

void redraw()
{
    display->clearDisplay();
    draw_cells();
    draw_keys();
}

void lock_cells(uint8_t codepoint)
{
    for(crypto_index_t& index : cell)
    {
        if (index.codepoint == codepoint)
        {
            index.state |= CELL_STATE_LOCKED;
        }
    }
}

void crypto_unlock_render(uint8_t* back_buffer)
{
    switch(state)
    {
        case CRYPTO_UNLOCK_ENTER:
            Log("Crypto Unlock entered");
            buttons = get_buttons();
            key_timer = 0;
            cycle_timer = 0;
            for(crypto_index_t& index : cell)
            {
                index.codepoint = 1 + (rand() % 254);
            }
            key_codepoints[UP_KEY] = 1 + (rand() % 254);
            key_codepoints[DOWN_KEY] = 1 + (rand() % 254);
            key_codepoints[SEL_KEY] = 1 + (rand() % 254);
            state = CRYPTO_UNLOCK_STEP;
            break;
        case CRYPTO_UNLOCK_STEP:
            {
                uint8_t nbtn = get_buttons();
                if (nbtn != buttons)
                {
                    if ((nbtn & BUTTON_UP_STATE_MASK)
                        && !(buttons & BUTTON_UP_STATE_MASK))
                    {
                        Log("CU UP: %c", (char)key_codepoints[UP_KEY]);
                        lock_cells(key_codepoints[UP_KEY]);
                    }

                    if ((nbtn & BUTTON_SEL_STATE_MASK)
                        && !(buttons & BUTTON_SEL_STATE_MASK))
                    {
                        Log("CU SEL: %c", key_codepoints[SEL_KEY]);
                        lock_cells(key_codepoints[SEL_KEY]);
                    }

                    if ((nbtn & BUTTON_DOWN_STATE_MASK)
                        && !(buttons & BUTTON_DOWN_STATE_MASK))
                    {
                        Log("CU DN: %c", key_codepoints[DOWN_KEY]);
                        lock_cells(key_codepoints[DOWN_KEY]);
                    }
                }
                buttons = nbtn;

                if ((millis() - cycle_timer) > CYCLE_RATE_MS)
                {
                    cycle_timer = millis();
                    codepoint_set.clear();
                    // Cycle all non-locked cells
                    uint16_t unlock_count = 0;
                    for(crypto_index_t& index : cell)
                    {
                        // Check if cell was locked
                        if (!(index.state & CELL_STATE_LOCKED))
                        {
                            index.codepoint = (rand() % 254) + 1;
                            codepoint_set.push_back(index.codepoint);
                            unlock_count++;
                        }
                    }

                    if (unlock_count == 0)
                    {
                        state = CRYPTO_UNLOCK_SUCCESS;
                    }
                }

                if ((millis() - key_timer) > KEY_ROTATE_MS)
                {
                    key_timer = millis();
                    // Pick 3 random codepoints that are different
                    key_codepoints[UP_KEY] = 0;
                    key_codepoints[SEL_KEY] = 0;
                    key_codepoints[DOWN_KEY] = 0;

                    if (codepoint_set.size() <= 3)
                    {
                        for (uint8_t i = UP_KEY; i < codepoint_set.size(); ++i)
                        {
                            key_codepoints[i] = codepoint_set[i];
                        }
                    }
                    else
                    {
                        while((key_codepoints[UP_KEY] == 0) ||
                            (key_codepoints[UP_KEY] == key_codepoints[SEL_KEY]) ||
                            (key_codepoints[UP_KEY] == key_codepoints[DOWN_KEY]))
                        {
                            key_codepoints[UP_KEY] = codepoint_set[rand() % codepoint_set.size()];
                        }

                        while((key_codepoints[SEL_KEY] == 0) ||
                            (key_codepoints[SEL_KEY] == key_codepoints[UP_KEY]) ||
                            (key_codepoints[SEL_KEY] == key_codepoints[DOWN_KEY]))
                        {
                            key_codepoints[SEL_KEY] = codepoint_set[rand() % codepoint_set.size()];
                        }

                        while((key_codepoints[DOWN_KEY] == 0) ||
                            (key_codepoints[DOWN_KEY] == key_codepoints[UP_KEY]) ||
                            (key_codepoints[DOWN_KEY] == key_codepoints[SEL_KEY]))
                        {
                            key_codepoints[DOWN_KEY] = codepoint_set[rand() % codepoint_set.size()];
                        }
                    }
                }
                redraw();
            }
            break;
        case CRYPTO_UNLOCK_SUCCESS:
            state = CRYPTO_UNLOCK_EXIT;
            break;
        case CRYPTO_UNLOCK_EXIT:
            pop_render_function();
            state = CRYPTO_UNLOCK_ENTER;
            break;
        default:
            pop_render_function();
            break;
    }
}
