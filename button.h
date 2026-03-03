#define AES_KEY_LEN 16

#define NUM_BUTTONS 5
static const uint8_t buttonPins[NUM_BUTTONS] = { M1_ON_BTN, M1_OFF_BTN, M2_ON_BTN, M2_OFF_BTN, STA_BTN };
static uint8_t buttonStates[NUM_BUTTONS] = { 1, 1, 1, 1, 1 };
static uint8_t lastButtonStates[NUM_BUTTONS] = { 1, 1, 1, 1, 1 };
static unsigned long lastDebounceTimes[NUM_BUTTONS] = { 0, 0, 0, 0, 0 };
static const unsigned long debounceDelay = 50UL;
unsigned long ackTimerMillis = 0;
uint8_t ackFailAtmp = 0;

void encryptNTx(const char *msg) {

  encryptData(msg);
  send_lora_data((uint8_t *)txBuffer, 32);
  //send_lora_data((uint8_t *)txBuffer, strlen(txBuffer));
  //DEBUG_PRINTN(txBuffer);
}



// Forward declaration — enterRemNodePairMode() is defined in pairRemoteNode.h
// which is included after button.h in the .ino
void enterRemNodePairMode();

// ── Runtime re-pair: hold M1_OFF (idx 1) + STA (idx 4) for 5 seconds ─────────
static unsigned long repairHoldStart = 0;
static bool          repairHolding   = false;

static inline void checkRepairCombo() {
  bool m1off_held = (digitalRead(M1_OFF_BTN) == LOW);
  bool sta_held   = (digitalRead(STA_BTN)    == LOW);

  if (m1off_held && sta_held) {
    if (!repairHolding) {
      repairHoldStart = millis();
      repairHolding   = true;
    } else if (millis() - repairHoldStart >= 5000UL) {
      repairHolding = false;
      watchdogReset();
      funcStaLRed();
      delay(300);
      funcLedReset();
      clearPeerSerial();
      enterRemNodePairMode();
    }
  } else {
    repairHolding = false;
  }
}

static inline void hwbuttonFunc() {
  checkRepairCombo();

  for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
    uint8_t reading = digitalRead(buttonPins[i]);
    if (reading != lastButtonStates[i]) lastDebounceTimes[i] = millis();
    if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;

        if ((buttonStates[i] == LOW) && (buttonEn[i] == ENABLED)) {  // restarting my controller
          watchdogReset();
          funcStaLBlue();
          buzBeep(BUZZ_NOR);
          lowPowerKick();
          delay(100);
          funcLedReset();
          while (digitalRead(buttonPins[i]) == LOW) {
            delay(1);  // small delay to avoid CPU hogging
          }

          buttonEn[0] = DISABLED;
          buttonEn[1] = DISABLED;
          buttonEn[2] = DISABLED;
          buttonEn[3] = DISABLED;
          buttonEn[4] = DISABLED;


          delay(100);

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
              //encryptNTx("[Z12345]");
              break;
          }

          //buttonEn[i] = DISABLED;

          msgTxd = 1;
          ackFailAtmp = 0;
          ackTimerMillis = millis();
        }
        // else if ((millis() <= 5000) && (buttonStates[3] == LOW) && (buttonStates[1] == LOW)) {
        //   watchdogDisableFun();
        //   savePeerSerial("42407197000000000000");
        //   funcStaLWhite();
        //   delay(300);
        //   funcLedReset();
        //   delay(300);
        //   funcStaLWhite();
        //   delay(300);
        //   funcLedReset();
        //   delay(2000);
        //   encryptNTx("[A?]");
        //   //encryptNTx("[A42407]");
        //   delay(2000);
        //   encryptNTx("[B19700]");
        //   delay(2000);
        //   encryptNTx("[C00123]");
        //   delay(2000);
        //   encryptNTx("[D45678]");
        //   delay(2000);
        //   encryptNTx("[Ex]");
        // }
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
        buttonEn[4] = ENABLED;
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