#ifndef DEBOUNCE_H_
#define DEBOUNCE_H_

#include <stdint.h>

#define BUTTON_UP  9
#define BUTTON_SEL 6
#define BUTTON_DN  5

#define DEBOUNCE_MS 50

#define BUTTON_UP_STATE_MASK 0x01
#define BUTTON_SEL_STATE_MASK 0x02
#define BUTTON_DOWN_STATE_MASK 0x04

#define BUTTON_RELEASED 1
#define BUTTON_PRESSED 0

void scan_buttons();
uint8_t get_buttons();

#endif // DEBOUNCE_H_
