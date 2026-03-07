#define AES_KEY_LEN 16

#define NUM_BUTTONS 3
static const uint8_t buttonPins[NUM_BUTTONS] = { M1_ON_BTN, M1_OFF_BTN, STA_BTN };
static uint8_t buttonStates[NUM_BUTTONS]     = { 1, 1, 1 };
static uint8_t lastButtonStates[NUM_BUTTONS] = { 1, 1, 1 };
static unsigned long lastDebounceTimes[NUM_BUTTONS] = { 0, 0, 0 };
static const unsigned long debounceDelay = 50UL;
unsigned long ackTimerMillis = 0;
uint8_t ackFailAtmp = 0;

void encryptNTx(const char *msg) {
  encryptData(msg);
  send_lora_data((uint8_t *)txBuffer, 32);
}

// Forward declaration — enterRemNodePairMode() is defined in pairRemoteNode.h
// which is included after button.h in the .ino
void enterRemNodePairMode();

static inline void hwbuttonFunc() {
  // Runtime re-pair combo REMOVED for v2-2 board.
  // Re-pair is triggered by boot combo only (M1_OFF + STA >= 5s in setup()).

  for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
    uint8_t reading = digitalRead(buttonPins[i]);
    if (reading != lastButtonStates[i]) lastDebounceTimes[i] = millis();
    if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;

        if ((buttonStates[i] == LOW) && (buttonEn[i] == ENABLED)) {
          watchdogReset();
          lowPowerKick();
          funcStaLBlue();       // blue LED ON
          buzBeep(100);         // buzzer ON — both LED + buzzer hold for 100 ms
          funcLedReset();       // blue LED OFF (buzzer already off after buzBeep)
          while (digitalRead(buttonPins[i]) == LOW) {
            delay(1);  // small delay to avoid CPU hogging
          }

          // Disable all buttons during TX
          for (uint8_t j = 0; j < NUM_BUTTONS; j++) buttonEn[j] = DISABLED;

          delay(100);

          switch (i) {
            case 0:
              encryptNTx("[1N]");
              break;
            case 1:
              encryptNTx("[1F]");
              break;
            case 2:
              encryptNTx("[S?]");
              break;
          }

          msgTxd = 1;
          ackFailAtmp = 0;
          ackTimerMillis = millis();
        }
      }
    }

    lastButtonStates[i] = reading;
  }
}

////////////////////////////////////////
void ackReception() {
  if (msgTxd) {
    if (millis() - ackTimerMillis > 5000) {
      if (ackFailAtmp >= 3) {
        noNetworkTone();
        funcStaLWhite();
        delay(300);
        funcLedReset();
        delay(300);
        funcStaLWhite();
        delay(300);
        funcLedReset();
        buttonEn[2] = ENABLED;   // re-enable STA only (index 2 in 3-button layout)
        msgTxd = 0;
      } else {
        funcStaLBlue();
        delay(300);
        funcLedReset();
        encryptNTx("[S?]");
        ackFailAtmp++;
      }
      ackTimerMillis = millis();
    }
  }
}
