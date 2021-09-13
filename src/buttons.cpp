#include <Arduino.h>

#include "buttons.h"
#include "utility.h"

#include <stdint.h>

static uint32_t last_change = 0;
static volatile uint8_t button_state = 0;

uint8_t get_buttons()
{
    return button_state;
}

void scan_buttons()
{
    uint8_t scan_state = 0;
    if (HIGH == digitalRead(BUTTON_UP))
    {
        scan_state |= BUTTON_UP_STATE_MASK;
    }
    else
    {
        scan_state &= ~BUTTON_UP_STATE_MASK;
    }

    if (HIGH == digitalRead(BUTTON_SEL))
    {
        scan_state |= BUTTON_SEL_STATE_MASK;
    }
    else
    {
        scan_state &= ~BUTTON_SEL_STATE_MASK;
    }

    if (HIGH == digitalRead(BUTTON_DN))
    {
        scan_state |= BUTTON_DOWN_STATE_MASK;
    }
    else
    {
        scan_state &= ~BUTTON_DOWN_STATE_MASK;
    }

    if (((millis() - last_change) > DEBOUNCE_MS)
        && (scan_state != button_state))
    {
        last_change = millis();
        button_state = scan_state;
    }
}
