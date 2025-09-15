#pragma once
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "debug.h"   



static volatile bool lp_wakeup_flag = false;
static unsigned long lp_last_activity = 0;
extern unsigned int __heap_start;
extern void *__brkval;

int freeMemory() {
  int free_memory;
  if ((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);
  return free_memory;
}

#if WATCHDOG
void watchdogInit() {
  // Wait until WDT is not locked
  while (WDT.STATUS & WDT_LOCK_bm);
  // Clear watchdog reset flag
  RSTCTRL.RSTFR = RSTCTRL_WDRF_bm; // Write 1 to clear
  // Unlock protected I/O
  CCP = CCP_IOREG_gc;
  // Enable watchdog with ~4s timeout
  WDT.CTRLA = WDT_PERIOD_4KCLK_gc;

   DEBUG_PRINTLN(F("Watchdog Timer Enabled: ~4-second timeout"));
}

void watchdogDisableFun() {
  // Wait until WDT is not locked
  while (WDT.STATUS & WDT_LOCK_bm);
  // Unlock protected I/O registers
  CCP = CCP_IOREG_gc;
  // Disable the watchdog timer by clearing CTRLA
  WDT.CTRLA = 0;
   DEBUG_PRINTLN(F("Watchdog Timer Disabled"));
}

// Reset watchdog (pet it)
static inline void watchdogReset() {
  __builtin_avr_wdr();
}

#else
inline void watchdogInit() {
  // watchdog disabled
}
inline void watchdogDisableFun() {
}
inline void watchdogReset() {
}
#endif

static void wakeup_isr() {
  lp_wakeup_flag = true;
}

static inline void lowPowerInit() {
  lp_last_activity = millis();
  pinMode(STA_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(STA_BTN), wakeup_isr, FALLING);
}

static inline void lowPowerKick() {
  lp_last_activity = millis();
}

static inline void lowPowerPoll() {
  if ((millis() - lp_last_activity) >= LP_TIMEOUT_MS) {
    // 1. Put LoRa to sleep
    llcc68_set_sleep(&llcc68_context, LLCC68_SLEEP_CFG_WARM_START);



    buzBeepNonBlocking(800);

    // 2. Disable watchdog before sleep
    watchdogDisableFun();

    // 3. Enter deep sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    cli();
    sei();
    sleep_cpu();

    // ==== wakes up here ====
    sleep_disable();

    // 4. Re-enable watchdog after wake
    watchdogInit();

    lp_wakeup_flag = true;
    lp_last_activity = millis();

     DEBUG_PRINTLN(F("System woke up from low-power mode"));
  }

  if (lp_wakeup_flag) {
    lp_wakeup_flag = false;
    llcc68_set_rx(&llcc68_context, 0xFFFFFF); // continuous RX
     DEBUG_PRINTLN(F("Low-power wakeup: Radio set to continuous RX"));
  }
}
