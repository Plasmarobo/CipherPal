#include "self_test.h"

#include "renderer.h"
#include "utility.h"

#include <algorithm>
#include <deque>

// A character is 6x8
// The LCD is 128 x 64
// The character display is therefore
#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8
#define CHARACTERS_PER_LINE 18
#define LINES 7
#define LOCK_IN_ACCELERATION 2
#define LOCKS_PER_ACC 16
#define UNLOCKED 0
#define ROTATION_RATE 100
#define NBLINKS 5
#define BLINK_TIME 250

#define COL(i) (i%CHARACTERS_PER_LINE)
#define ROW(i) (i/CHARACTERS_PER_LINE)

typedef enum
{
    SELF_TEST_ENTER,
    SELF_TEST_RUN,
    SELF_TEST_BLINK,
    SELF_TEST_EXIT
} self_test_state_t;

typedef struct
{
    uint8_t index;
    uint8_t value;
} lock_in_t;

// Each byte represents one column
static self_test_state_t state = SELF_TEST_ENTER;

void render_lock_in(std::deque<lock_in_t>& lock_in)
{
    for (uint16_t i = 0; i < CHARACTERS_PER_LINE * LINES; ++i)
    {
        uint8_t x = COL(lock_in[i].index) * (CHAR_WIDTH + 1);
        uint8_t y = ROW(lock_in[i].index) * (CHAR_HEIGHT + 1);

        if (lock_in[i].value == UNLOCKED)
        {
            display->drawChar(x, y, (char)1 + (rand() % 254), MONOOLED_WHITE, MONOOLED_BLACK, 1);
        }
        else
        {
            display->drawChar(x, y, (char)lock_in[i].value, MONOOLED_WHITE, MONOOLED_BLACK, 1);
        }
    }
}

void self_test_render(uint8_t* back_buffer)
{
    static std::deque<lock_in_t> lock_in;
    static uint32_t lock_in_rate = 2;
    static uint32_t last_lock_in = 0;
    static uint32_t last_rotate = 0;
    static uint16_t locks = 0;
    static uint8_t blinks = 0;
    switch(state)
    {
        case SELF_TEST_ENTER:
            Log("Self test entered");
            lock_in.resize(CHARACTERS_PER_LINE * LINES);
            for(uint16_t i = 0; i < CHARACTERS_PER_LINE * LINES; ++i)
            {
                lock_in[i].index = i;
                lock_in[i].value = UNLOCKED;
            }
            std::random_shuffle(lock_in.begin(), lock_in.end());
            last_lock_in = millis();
            last_rotate = millis();
            lock_in_rate = 2;
            locks = 0;
            state = SELF_TEST_RUN;
            break;
        case SELF_TEST_RUN:
            // Each character that remains in the lock-in deque
            // should randomly rotate
            if ((millis() - last_rotate) > ROTATION_RATE)
            {
                display->clearDisplay();
                last_rotate = millis();
                render_lock_in(lock_in);
            }
            if ((millis() - last_lock_in) > lock_in_rate)
            {
                last_lock_in = millis();
                ++locks;
                if (locks > LOCKS_PER_ACC)
                {
                    lock_in_rate *= LOCK_IN_ACCELERATION;
                    locks = 0;
                }
                uint16_t i = 0;
                while(i < CHARACTERS_PER_LINE * LINES)
                {
                    if (lock_in[i].value == UNLOCKED)
                    {
                        lock_in[i].value = 1 + (rand() % 254);
                        break;
                    }
                    ++i;
                }
                if (i >= CHARACTERS_PER_LINE * LINES)
                {
                    state = SELF_TEST_BLINK;
                    last_rotate = millis();
                }
            }
            break;
        case SELF_TEST_BLINK:
            if ((millis() - last_rotate) > BLINK_TIME)
            {
                last_rotate = millis();
                display->clearDisplay();
                if ((blinks % 2) == 0)
                {
                    render_lock_in(lock_in);
                }
                ++blinks;
                if (blinks >= (2*NBLINKS))
                {
                    state = SELF_TEST_EXIT;
                }
            }
            break;
        case SELF_TEST_EXIT:
            pop_render_function();
            state = SELF_TEST_ENTER;
            break;
        default:
            pop_render_function();
            break;
    }
}
