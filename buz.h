// Buzzer: short blocking beep only for small durations
static inline void buzBeep(uint16_t ms) {
  digitalWrite(BUZ, LOW);
  delay(ms);
  digitalWrite(BUZ, HIGH);
}
