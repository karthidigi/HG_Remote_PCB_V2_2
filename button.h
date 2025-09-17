#define AES_KEY_LEN 16

#define NUM_BUTTONS 5
static const uint8_t buttonPins[NUM_BUTTONS] = { M1_ON_BTN, M1_OFF_BTN, M2_ON_BTN, M2_OFF_BTN, STA_BTN };
static uint8_t buttonStates[NUM_BUTTONS] = { 1, 1, 1, 1, 1 };
static uint8_t lastButtonStates[NUM_BUTTONS] = { 1, 1, 1, 1, 1 };
static unsigned long lastDebounceTimes[NUM_BUTTONS] = { 0, 0, 0, 0, 0 };
static const unsigned long debounceDelay = 50UL;

void encryptNTx(const char *msg) {

  encryptData(msg);
  send_lora_data((uint8_t *)txBuffer, 32);
  //send_lora_data((uint8_t *)txBuffer, strlen(txBuffer));
  //DEBUG_PRINTN(txBuffer);
}



static inline void hwbuttonFunc() {
  for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
    uint8_t reading = digitalRead(buttonPins[i]);
    if (reading != lastButtonStates[i]) lastDebounceTimes[i] = millis();
    if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;
        if (buttonStates[i] == LOW && buttonEn[i] == HIGH) {  // restarting my controller
          watchdogReset();
          funcStaLBlue();
          buzBeep(BUZZ_NOR);
          lowPowerKick();
          delay(100);
          funcLedReset();
          buttonEn[0] = 0;
          buttonEn[1] = 0;
          buttonEn[2] = 0;
          buttonEn[3] = 0;
          while (digitalRead(buttonPins[i]) == LOW) {
            delay(1);  // small delay to avoid CPU hogging
          }
          switch (i) {
            case 0:
              encryptNTx("[1N]");
              break;
            case 1:
              encryptNTx("[1F]");
              break;
            case 2:
              encryptNTx("[2N]");
              break;
            case 3:
              encryptNTx("[2F]");
              break;
            case 4:
              encryptNTx("[S?]");
              break;
          }
          msgTxd = 1;
          ackTimerMillis = millis();
        }
      }
    }

    lastButtonStates[i] = reading;
  }
}