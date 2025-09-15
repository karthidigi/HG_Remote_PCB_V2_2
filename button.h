#define AES_KEY_LEN 16

#define BUZZ_DUR 120

// #define NUM_BUTTONS 4
// static const int buttonPins[NUM_BUTTONS] = { M1_ON_BTN, M1_OFF_BTN, M2_OFF_BTN, STA_BTN };

#define NUM_BUTTONS 5
static const int buttonPins[NUM_BUTTONS] = { M1_ON_BTN, M1_OFF_BTN, M2_OFF_BTN, STA_BTN, M2_ON_BTN };

static uint8_t buttonStates[NUM_BUTTONS];
static uint8_t lastButtonStates[NUM_BUTTONS];
static unsigned long lastDebounceTimes[NUM_BUTTONS];
static const unsigned long debounceDelay = 50UL;



void encryptNTx(const char *msg){

encryptData(msg);
send_lora_data((uint8_t *)txBuffer, strlen(txBuffer));
//DEBUG_PRINTN(txBuffer);

}


static inline void hwbuttonInit() {
  for (int i = 0; i < NUM_BUTTONS; ++i) {
    buttonStates[i] = HIGH;
    lastButtonStates[i] = HIGH;
    lastDebounceTimes[i] = 0;
  }
}

static inline void hwbuttonFunc() {
  for (int i = 0; i < NUM_BUTTONS; ++i) {
    int reading = digitalRead(buttonPins[i]);
    if (reading != lastButtonStates[i]) lastDebounceTimes[i] = millis();
    if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;
        if (buttonStates[i] == LOW) {  // pressed
          switch (i) {
            case 0:
              //funcM1LGreen();
              encryptNTx("[M1ON]");
              buzBeep(BUZZ_DUR);
              funcLedReset();
              //watchdogReset();
              lowPowerKick();
              break;
            case 1:
              //funcM1LRed();
              encryptNTx("[M1OFF]");
              buzBeep(BUZZ_DUR);
              funcLedReset();
              //watchdogReset();
              lowPowerKick();
              break;
            case 2:
              //funcM2LRed();
              encryptNTx("[M1OFF]");
              buzBeep(BUZZ_DUR);
              funcLedReset();
              //watchdogReset();
              lowPowerKick();
              break;
            case 3:
              //funcStaLBlue();
              encryptNTx("[STB]");
              buzBeep(BUZZ_DUR);
              funcLedReset();
              //watchdogReset();
              lowPowerKick();
              break;
            case 4:
              //funcM2LGreen();
              encryptNTx("[M1ON]");
              buzBeep(BUZZ_DUR);
              funcLedReset();
              //watchdogReset();
              lowPowerKick();
              break;
          }
        }
      }
    }
    lastButtonStates[i] = reading;
  }
}
