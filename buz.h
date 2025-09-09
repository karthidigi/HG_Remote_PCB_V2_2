//////////////////////////////////////////////////
void buzBeep(int x) {
  digitalWrite(BUZ, LOW);
  delay(x);
  digitalWrite(BUZ, HIGH);
}

void buzStbAlert(){

  static uint32_t lastUpdate = 0;
  uint32_t now = millis();

  if (now - lastUpdate >= 60000) {
    lastUpdate = now;
    funcLedReset();
    buzBeep(500);
    delay(200);
    buzBeep(200);
  }

}
