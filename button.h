const int NUM_BUTTONS = 5;

// Button and LED pin numbers
const int buttonPins[NUM_BUTTONS] = {M1_ON_BTN, M1_OFF_BTN, M2_ON_BTN, M2_OFF_BTN, STA_BTN};

// States
int buttonStates[NUM_BUTTONS]      = {HIGH};  // pulled HIGH initially
int lastButtonStates[NUM_BUTTONS]  = {HIGH};

// Debounce timing
unsigned long lastDebounceTimes[NUM_BUTTONS] = {0};
const unsigned long debounceDelay = 50;


void hwbuttonPin() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int reading = digitalRead(buttonPins[i]);

    // If state changed
    if (reading != lastButtonStates[i]) {
      lastDebounceTimes[i] = millis();
    }

    if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;
        //Serial.println(i);

        // LOW means pressed (due to INPUT_PULLUP)
        if (buttonStates[i] == LOW) {
          // Call corresponding function
          switch (i) {
            case 0: 
            funcM1LGreen();
            loraTxFunc("M1",1);
            buzBeep(200); 
            funcLedReset();
            break;
            case 1:
            funcM1LRed();
            loraTxFunc("M1",0);
            buzBeep(200);
            funcLedReset();
            break;
            case 2: 
            funcM2LGreen();
            loraTxFunc("M2",1);
            buzBeep(200);
            funcLedReset();
            break;
            case 3: 
            funcM2LRed();
            loraTxFunc("M2",0);
            buzBeep(200);
            funcLedReset();
            break;
            case 4: 
            funcStaLBlue();
            loraTxFunc("STB", 1);
            buzBeep(200);
            funcLedReset();
            break;
          }
        }
      }
    }

    lastButtonStates[i] = reading;
  }
}
