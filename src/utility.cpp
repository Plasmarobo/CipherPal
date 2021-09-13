#include "utility.h"

#include <stdarg.h>

#define BUFFER_LENGTH 128

void Log(const __FlashStringHelper *format, ...)
{
  char buffer[BUFFER_LENGTH];
  va_list args;
  va_start (args, format);
#ifdef __AVR__
  vsnprintf_P(buffer, sizeof(buffer), (const char *)format, args);
#else
  vsnprintf(buffer, sizeof(buffer), (const char *)format, args);
#endif
  va_end(args);
  Serial.print(buffer);
  Serial.flush();
}

void Log(const char* format, ...)
{
    char buffer[BUFFER_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, BUFFER_LENGTH, format, args);
    va_end(args);
    Serial.println(buffer);
    Serial.flush();
}
