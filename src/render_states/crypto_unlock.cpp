#include "crypto_unlock.h"

#include "buttons.h"
#include "register_read.h"
#include "renderer.h"
#include "utility.h"

#include <algorithm>
#include <map>
#include <vector>
#include <set>

#define KEY_ROTATE_MS 5000
#define CYCLE_RATE_MS 1000
#define FLASH_RATE_MS 500
#define CYCLE_COUNT 6
#define CELL_STATE_LOCKED 0x01
#define CHARACTERS_PER_LINE 11
#define UP_KEY 0
#define DOWN_KEY 2
#define SEL_KEY 1
#define KEY_COUNT 3
#define NBLINKS 5
#define BLINK_TIME 250

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
    CRYPTO_UNLOCK_BLINK,
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
static std::map<uint8_t, uint8_t> codepoint_set;
static std::set<uint8_t> unlocked_set;
static uint8_t blinks;

void draw_cells()
{
    // Reduce tick to 0 or 1
    uint8_t tick = (millis() % (2 * FLASH_RATE_MS)) > FLASH_RATE_MS;
    for(uint16_t i = 0; i < CELLS; ++i)
    {
        uint8_t x = 5 + ((i % CHARACTERS_PER_LINE) * 11);
        uint8_t y = 4 + ((i / CHARACTERS_PER_LINE) * 17);
        display->drawChar(x,
                          y,
                          (char)cell[i].codepoint,
                          ((cell[i].state & CELL_STATE_LOCKED) && tick) ? MONOOLED_BLACK : MONOOLED_WHITE,
                          ((cell[i].state & CELL_STATE_LOCKED) && tick) ? MONOOLED_WHITE : MONOOLED_BLACK,
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
    for(uint8_t i = 0; i < CELLS; ++i)
    {
        crypto_index_t& index = cell[i];
        if ((index.codepoint != 0) &&
            (index.codepoint == codepoint))
        {
            index.state |= CELL_STATE_LOCKED;
            codepoint_set[codepoint]--;
            if (codepoint_set[codepoint] == 0)
            {
                codepoint_set.erase(codepoint);
            }
            unlocked_set.erase(i);
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
            unlocked_set.clear();
            codepoint_set.clear();
            for(uint8_t i = 0; i < CELLS; ++i)
            {
                crypto_index_t& index = cell[i];
                index.codepoint = 1 + (rand() % 254);
                codepoint_set[index.codepoint]++;
                index.state = 0x00;
                unlocked_set.insert(i);
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
                    // Cycle all non-locked cells
                    std::set<uint8_t> indices_to_shift;
                    if (unlocked_set.size() <= CYCLE_COUNT)
                    {
                        indices_to_shift = unlocked_set;
                    }
                    else
                    {
                        std::set<uint8_t> possible_indices = unlocked_set;
                        while (indices_to_shift.size() < CYCLE_COUNT)
                        {
                            // Pick a random element in unlocked set
                            auto iter = possible_indices.begin();
                            std::advance(iter, rand() % possible_indices.size());
                            indices_to_shift.insert(*iter);
                            possible_indices.erase(iter);
                        }
                    }
                    if (indices_to_shift.size() == 0)
                    {
                        cycle_timer = millis();
                        register_unlocked = true;
                        state = CRYPTO_UNLOCK_BLINK;
                        break;
                    }
                    for(uint16_t i : indices_to_shift)
                    {
                        // Check if cell was locked
                        crypto_index_t& index = cell[i];
                        // If no other elements share a codepoint, remove it from the set
                        uint8_t count = 0;
                        codepoint_set[index.codepoint]--;
                        if (codepoint_set[index.codepoint] == 0)
                        {
                            codepoint_set.erase(index.codepoint);
                        }
                        index.codepoint = (rand() % 254) + 1;
                        codepoint_set[index.codepoint]++;
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
                        auto iter = codepoint_set.begin();
                        for (uint8_t i = UP_KEY; i < codepoint_set.size(); ++i)
                        {
                            key_codepoints[i] = (iter++)->first;
                            if (iter == codepoint_set.end())
                            {
                                iter = codepoint_set.begin();
                            }
                        }
                    }
                    else
                    {
                        while((key_codepoints[UP_KEY] == 0) ||
                            (key_codepoints[UP_KEY] == key_codepoints[SEL_KEY]) ||
                            (key_codepoints[UP_KEY] == key_codepoints[DOWN_KEY]))
                        {
                            auto it = codepoint_set.begin();
                            std::advance(it, rand() % codepoint_set.size());
                            key_codepoints[UP_KEY] = it->first;
                        }

                        while((key_codepoints[SEL_KEY] == 0) ||
                            (key_codepoints[SEL_KEY] == key_codepoints[UP_KEY]) ||
                            (key_codepoints[SEL_KEY] == key_codepoints[DOWN_KEY]))
                        {
                            auto it = codepoint_set.begin();
                            std::advance(it, rand() % codepoint_set.size());
                            key_codepoints[SEL_KEY] = it->first;
                        }

                        while((key_codepoints[DOWN_KEY] == 0) ||
                            (key_codepoints[DOWN_KEY] == key_codepoints[UP_KEY]) ||
                            (key_codepoints[DOWN_KEY] == key_codepoints[SEL_KEY]))
                        {
                            auto it = codepoint_set.begin();
                            std::advance(it, rand() % codepoint_set.size());
                            key_codepoints[DOWN_KEY] = it->first;
                        }
                    }
                }
                redraw();
            }
            break;
        case CRYPTO_UNLOCK_BLINK:
            if ((millis() - cycle_timer) > BLINK_TIME)
            {
                cycle_timer = millis();
                display->clearDisplay();
                if ((blinks % 2) == 0)
                {
                    redraw();
                }
                ++blinks;
                if (blinks >= (2*NBLINKS))
                {
                    state = CRYPTO_UNLOCK_EXIT;
                }
            }
            break;
        case CRYPTO_UNLOCK_EXIT:
            blinks = 0;
            pop_render_function();
            state = CRYPTO_UNLOCK_ENTER;
            break;
        default:
            pop_render_function();
            break;
    }
}
