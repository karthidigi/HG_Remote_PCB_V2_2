#include <avr/sleep.h>
#include <avr/interrupt.h>

static volatile bool lp_wakeup_flag = false;
static unsigned long lp_last_activity = 0;
extern unsigned int __heap_start;
extern void *__brkval;

bool lp_wkup_stbTx = 0;

// ISR – native PORT interrupt (ATtiny1606 PC2 = STA_BTN)
// PORTC_PORT_vect fires for any pin in PORT C.
ISR(PORTC_PORT_vect) {
  if (PORTC.INTFLAGS & PIN2_bm) {
    PORTC.INTFLAGS = PIN2_bm;   // clear flag (write 1 to clear)
    lp_wakeup_flag = true;
  }
}

static inline void lowPowerInit() {
  lp_last_activity = millis();
  // PC2 (STA_BTN) falling-edge interrupt — native PORT register
  // PULLUPEN preserved: hwPinInit() set INPUT_PULLUP; we keep it here explicitly.
  PORTC.DIRCLR   = PIN2_bm;                               // PC2 as input
  PORTC.PIN2CTRL = PORT_ISC_FALLING_gc | PORT_PULLUPEN_bm; // falling-edge + pull-up
}

static inline void lowPowerKick() {
  lp_last_activity = millis();
}

static inline void enterSleep() {
  // 1. Put SX1268 to sleep (warm start: retains calibration, faster wake)
  SX1268_setSleep(SX1268_SLEEP_WARM_START);
  // buzBeep(100);
  // delay(200);
  // buzBeep(100);
  // delay(200);
  // buzBeep(100);

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
  buttonEn[4] = ENABLED;
  lp_wakeup_flag = true;
  lp_wkup_stbTx = true;
  lp_last_activity = millis();
  //DEBUG_PRINTN(F("System woke up from low-power mode"));
}

static inline void lowPowerPoll() {
  if ((millis() - lp_last_activity) >= LP_TIMEOUT_MS) {
    enterSleep();
  }
  if (lp_wakeup_flag) {
    // Wake SX1268 from sleep: assert NSS low briefly (SX126x datasheet §4.1.3)
    digitalWrite(SX1268_NSS, LOW);
    delayMicroseconds(200);
    digitalWrite(SX1268_NSS, HIGH);
    lp_wakeup_flag = false;
  }
}
