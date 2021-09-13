#include <Arduino.h>

#include "renderer.h"

#include "images.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include <stack>
#include <stdint.h>

static uint8_t _lcd_buffer[LCD_HEIGHT * BYTES_PER_LINE];
static std::stack<render_function_t> render_state;
Adafruit_SH1107* display;

void blitBuffer();

void blit_buffer(uint8_t* buffer)
{
    display->drawBitmap(0,0, buffer, LCD_WIDTH, LCD_HEIGHT, MONOOLED_WHITE);
}

void render_init()
{
    display = new Adafruit_SH1107(64, 128, &Wire);
    // text display tests
    display->begin(0x3C, true); // Address 0x3C default
    display->cp437(true);
    display->setRotation(1);
    display->setTextSize(1);
    display->setTextColor(SH110X_WHITE);
}

void push_render_function(render_function_t func)
{
    render_state.push(func);
}

void pop_render_function()
{
    if (!render_state.empty())
    {
        render_state.pop();
    }
}

void render()
{
    if (!render_state.empty())
    {
        (render_state.top())(_lcd_buffer);
    }
    display->display();
}
