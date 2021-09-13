#include <Arduino.h>

#include "buttons.h"
#include "renderer.h"
#include "render_states/splash_screen.h"
#include "render_states/main_menu.h"

#include <HardwareSerial.h>

void setup() {
  Serial.begin(115200);
  Serial.println("128x64 OLED FeatherWing test");
  Serial.println("OLED begun");

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_SEL, INPUT_PULLUP);
  pinMode(BUTTON_DN, INPUT_PULLUP);

  render_init();

  push_render_function(&main_menu_render);
  push_render_function(&splash_screen_render);
}

void loop() {
  scan_buttons();
  render();
  yield();
}
