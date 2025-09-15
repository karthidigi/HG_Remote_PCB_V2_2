// ---------- FILE: debug.h ----------
#pragma once
#include "zSettings.h"

#ifdef SERIAL_DEBUG
  #define DEBUG_BEGIN() debugSerial.begin(SERIAL_BAUD); debugSerial.setTimeout(SERIAL_TIMEOUT)
  #define DEBUG_PRINT(x) debugSerial.print(x)
  #define DEBUG_PRINTN(x) debugSerial.println(x)
  #define DEBUG_PRINTLN(x) debugSerial.println(x)
#else
  #define DEBUG_BEGIN()
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTN(x)
  #define DEBUG_PRINTLN(x)
#endif

#define debugSerial Serial

static inline void hwSerialInit() {
  #ifdef SERIAL_DEBUG
    debugSerial.begin(SERIAL_BAUD);
    debugSerial.setTimeout(SERIAL_TIMEOUT);
  #endif
}

// Buzzer: short blocking beep only for small durations
static inline void buzBeepNonBlocking(uint16_t ms) {
  digitalWrite(BUZ, LOW);
  delay(ms);
  digitalWrite(BUZ, HIGH);
}
