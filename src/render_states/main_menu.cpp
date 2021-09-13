#include "main_menu.h"

#include "buttons.h"
#include "crypto_unlock.h"
#include "renderer.h"
#include "self_test.h"
#include "utility.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

bool getPressEvent(uint8_t button, uint8_t mask);
void rotateMenu(int8_t direction);
void update_menu();
void draw_menu();

typedef struct {
    render_function_t state;
    const char* name;
} menu_item_t;

#define MAX_MENU_ITEMS 4

void register_read_render(uint8_t* back_buffer)
{
    UNUSED(back_buffer);
    pop_render_function();
}

void buffer_deconstruct_render(uint8_t* back_buffer)
{
    UNUSED(back_buffer);
    pop_render_function();
}

static menu_item_t self_test = {
    .state = self_test_render,
    .name = "SELF TEST"
};

static menu_item_t crypto_unlock = {
    .state = crypto_unlock_render,
    .name = "CRYPTO UNLOCK"
};

static menu_item_t register_read = {
    .state = register_read_render,
    .name = "REGISTER READ"
};

static menu_item_t buffer_deconstruct = {
    .state = buffer_deconstruct_render,
    .name = "BUFFER DECON"
};

static menu_item_t* menu[MAX_MENU_ITEMS] = {
    &self_test,
    &crypto_unlock,
    &register_read,
    &buffer_deconstruct
};

static int8_t selected = 0;
static const int16_t entry_height = 15;
static const int16_t entry_width = 128;
static const int16_t entry_pad = 1;
static const int16_t entry_margin = 1;
static const int16_t entry_border = 1;

static uint8_t buttons = 0xFF;

void rotateMenu(int8_t direction)
{
    selected += direction;
    if (selected >= MAX_MENU_ITEMS)
    {
        selected = 0;
    }
    if (selected < 0)
    {
        selected = MAX_MENU_ITEMS - 1;
    }
}

int16_t drawMenuItem(menu_item_t* item, int16_t x, int16_t y, bool selected)
{
    display->setCursor(
        x+entry_margin+entry_border+entry_pad,
        y+entry_margin+entry_border+entry_pad);
    display->print(item->name);

    if (selected)
    {
        display->drawRect(
            x+entry_margin,
            y+entry_margin,
            entry_width-(2*entry_pad)-(entry_margin*2),
            entry_height-(2*entry_pad)-(entry_margin*2),
            MONOOLED_WHITE);
    }
    return entry_margin + entry_pad + entry_height + 1;
}

void update_menu()
{
    uint8_t nbtn = get_buttons();
    if (buttons != nbtn)
    {
        if ((nbtn & BUTTON_UP_STATE_MASK)
            && !(buttons & BUTTON_UP_STATE_MASK))
        {
            // Up rising edge
            rotateMenu(-1);
        }
        if ((nbtn & BUTTON_DOWN_STATE_MASK)
            && !(buttons & BUTTON_DOWN_STATE_MASK))
        {
            // Down rising edge
            rotateMenu(1);
        }
        if ((nbtn & BUTTON_SEL_STATE_MASK)
            && !(buttons & BUTTON_SEL_STATE_MASK))
        {
            // Sel rising edge
            push_render_function(menu[selected]->state);
        }
        buttons = nbtn;
    }
}

void draw_menu()
{
    // Draw previous
    uint8_t current = selected - 1;
    if (current > MAX_MENU_ITEMS)
    {
        current = 0;
    }
    uint8_t y = 1;
    for(uint8_t i = 0; i < MAX_MENU_ITEMS; ++i)
    {
        if (menu[i] == NULL)
        {
            break;
        }

        drawMenuItem(menu[i], 0, y, selected == i);
        y += entry_height;
    }
}

void main_menu_render(uint8_t* buffer)
{
    display->clearDisplay();
    update_menu();
    draw_menu();
}
