////////////////////////////////////////
static uint8_t buttonEn[5] = { 1, 1, 1, 1, 1 };
bool msgTxd = 0;
unsigned long ackTimerMillis = 0;
////////////////////////////////////////
void ackReception() {
  if (msgTxd) {
    if (millis() - ackTimerMillis > 10000) {
      noNetworkTone();
      funcStaLWhite();
      delay(300);
      funcLedReset();
      delay(300); 
      funcStaLWhite();
      delay(300);
      funcLedReset();
      msgTxd = 0;
    }
  }
}