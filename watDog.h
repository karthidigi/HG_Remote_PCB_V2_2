bool wdtEnabled = false;

#if WATCHDOG
void watchdogInit() {
  while (WDT.STATUS & WDT_LOCK_bm);        // wait until unlocked
  RSTCTRL.RSTFR = RSTCTRL_WDRF_bm;         // clear watchdog reset flag
  CCP = CCP_IOREG_gc;                      // unlock protected I/O
  WDT.CTRLA = WDT_PERIOD_4KCLK_gc;         // enable watchdog (~4s)
  //DEBUG_PRINTN(F("Watchdog Timer Enabled: ~4-second timeout"));
}


//////////////////////
void watchdogDisableFun() {
  while (WDT.STATUS & WDT_LOCK_bm);
  CCP = CCP_IOREG_gc;
  WDT.CTRLA = 0;                           // disable watchdog
  //DEBUG_PRINTN(F("Watchdog Timer Disabled"));
}


//////////////////////
static inline void watchdogReset() {
  __builtin_avr_wdr();                     // reset/pet watchdog
}

#else
inline void watchdogInit() {}
inline void watchdogDisableFun() {}
inline void watchdogReset() {}
#endif