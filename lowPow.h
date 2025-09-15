#include <avr/sleep.h>
#include <avr/interrupt.h>

static volatile bool lp_wakeup_flag = false;
static unsigned long lp_last_activity = 0;
extern unsigned int __heap_start;
extern void *__brkval;

// ISR for button wakeup (called by attachInterrupt)
static void wakeup_isr() {
  lp_wakeup_flag = true;
}

static inline void lowPowerInit() {
  lp_last_activity = millis();
 //pinMode(STA_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(STA_BTN), wakeup_isr, FALLING);
}

static inline void lowPowerKick() {
  lp_last_activity = millis();
}

static inline void enterSleep() {
  // 1. Put LoRa to sleep
  llcc68_set_sleep(&llcc68_context, LLCC68_SLEEP_CFG_WARM_START);
  buzBeep(800);

  // 2. Disable watchdog before sleep
  watchdogDisableFun();

  // 3. Enter deep sleep
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sei();            // enable interrupts before sleeping
  sleep_cpu();      // go to sleep
  sleep_disable();  // resumes here after wake

  // 4. Re-enable watchdog after wake
  watchdogInit();

  lp_wakeup_flag = true;
  lp_last_activity = millis();
  //DEBUG_PRINTN(F("System woke up from low-power mode"));
}

static inline void lowPowerPoll() {
  if ((millis() - lp_last_activity) >= LP_TIMEOUT_MS) {
    enterSleep();
  }

  if (lp_wakeup_flag) {
    lp_wakeup_flag = false;
    llcc68_set_rx(&llcc68_context, 0xFFFFFF);  // continuous RX
    //DEBUG_PRINTN(F("Low-power wakeup: Radio set to continuous RX"));
  }
}
