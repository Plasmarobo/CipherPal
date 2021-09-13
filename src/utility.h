#ifndef UTILITY_H
#define UTILITY_H

#include <Arduino.h>

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

#define UNUSED(x) ((void)(x))

void Log(const __FlashStringHelper *fmt, ... );
void Log(const char* format, ...);

#endif //UTILITY_H
