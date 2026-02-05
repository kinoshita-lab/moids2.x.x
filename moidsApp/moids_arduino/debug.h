#pragma once

#ifdef DEBUG
#define DEBUG_PRINTF(fmt, ...) printf(fmt, __VA_ARGS__);
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINTF(fmt, ...)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
