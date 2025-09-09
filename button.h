const int NUM_BUTTONS = 5;

// Button and LED pin numbers
const int buttonPins[NUM_BUTTONS] = { M1_ON_BTN, M1_OFF_BTN, M2_ON_BTN, M2_OFF_BTN, STA_BTN };

// States
int buttonStates[NUM_BUTTONS] = { HIGH };  // pulled HIGH initially
int lastButtonStates[NUM_BUTTONS] = { HIGH };

// Debounce timing
unsigned long lastDebounceTimes[NUM_BUTTONS] = { 0 };
const unsigned long debounceDelay = 50;


//Example: Send custom data on demand (e.g., triggered by a button or condition)

void sendLoraData(const String& msg) {
  msg = "[" + msg + "]";
  const char* plainText = msg.c_str();  // Correct conversion
  String serialKey = getChipSerial();

  uint16_t rnd = rand() & 0xFFFF;

  rnd = 0x1111;
  Serial.print("Random 16-bit: ");
  Serial.println(rnd, HEX);

  uint8_t key[KEY_LEN] = { 0 };
  const char* sk_str = serialKey.c_str();
  size_t slen = strlen(sk_str);
  const char* last12 = (slen >= 12) ? sk_str + slen - 12 : sk_str;
  size_t offset = KEY_LEN - 12 - 2;
  //Print the last 12 charater of serialKey
  Serial.print("Last 12 chars of serialKey: ");
  Serial.println(last12);
  memcpy(key + offset, last12, strlen(last12));
  key[KEY_LEN - 2] = (uint8_t)(rnd >> 8);
  key[KEY_LEN - 1] = (uint8_t)(rnd & 0xFF);

  Serial.print("Derived key: ");
  for (int i = 0; i < KEY_LEN; i++) {
    if (key[i] < 0x10) Serial.print("0");
    Serial.print(key[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  //  Pick a random nonce index from 0 to 31
  int idx = rand() % 32;
  idx = 1;
  Serial.print("Using nonce index: ");
  Serial.println(idx);

  uint8_t nonce[BLOCK_SIZE];
  memcpy(nonce, nonces[idx], BLOCK_SIZE);

  Serial.print("Nonce bytes: ");
  for (int i = 0; i < BLOCK_SIZE; i++) {
    if (nonce[i] < 0x10) Serial.print("0");
    Serial.print(nonce[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("Before encryption: ");
  Serial.println(plainText);

  char txBuffer[MAX_MESSAGE_LEN * 2 + 1] = { 0 };
  strcpy(txBuffer, encryptStringWithKeyNonce(plainText, key, nonce));

  Serial.print("Encrypted hex: ");
  Serial.println(txBuffer);
  Serial.print("Encrypted length: ");
  Serial.println(strlen(txBuffer));

  char* decrypted = decryptStringWithKeyNonce(txBuffer, key, nonce);
  Serial.print("Decrypted text: ");
  Serial.println(decrypted);
  Serial.println("------------------------");

  size_t encryptedLen = strlen(txBuffer);
  // Queue data for LoRa transmission
  send_lora_data((uint8_t*)txBuffer, encryptedLen);
  // Optionally call to skip or stop next transmission cycles
}


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
              sendLoraData("M1ON");
              //loraTxFunc("M1",1);
              buzBeep(200);
              funcLedReset();
              break;
            case 1:
              funcM1LRed();
              sendLoraData("M1OFF");
              //loraTxFunc("M1",0);
              buzBeep(200);
              funcLedReset();
              break;
            case 2:
              funcM2LGreen();
              sendLoraData("M2ON");
              //loraTxFunc("M2",1);
              buzBeep(200);
              funcLedReset();
              break;
            case 3:
              funcM2LRed();
              sendLoraData("M2OFF");
              //loraTxFunc("M2",0);
              buzBeep(200);
              funcLedReset();
              break;
            case 4:
              funcStaLBlue();
              sendLoraData("STB");
              //loraTxFunc("STB", 1);
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
