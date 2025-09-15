#pragma once

#define AES_KEY_LEN 16
#define HEX_BUFFER_LEN (MAX_MESSAGE_LEN * 2 + 8)

// #define NUM_BUTTONS 4
// static const int buttonPins[NUM_BUTTONS] = { M1_ON_BTN, M1_OFF_BTN, M2_OFF_BTN, STA_BTN };

#define NUM_BUTTONS 5
static const int buttonPins[NUM_BUTTONS] = { M1_ON_BTN, M1_OFF_BTN, M2_OFF_BTN, STA_BTN, M2_ON_BTN };



static uint8_t buttonStates[NUM_BUTTONS];
static uint8_t lastButtonStates[NUM_BUTTONS];
static unsigned long lastDebounceTimes[NUM_BUTTONS];
static const unsigned long debounceDelay = 50UL;

static char txBuffer[HEX_BUFFER_LEN + 4];

static inline void sendLoraData(const char *msg) {

  //DEBUG_PRINTLN(F("----- AES TEST -----"));
  //DEBUG_PRINT(F("Msg: "));
  //DEBUG_PRINTLN(msg);

  // Get chip serial
  char HwSerialKey[21];
  // Read peer serial from EEPROM instead of chip serial
  readPeerSerial(HwSerialKey, sizeof(HwSerialKey));
  //DEBUG_PRINT(F("Peer Serial: "));
  //DEBUG_PRINTLN(HwSerialKey);
  // Build AES key
  uint8_t key[KEY_LEN] = { 0 };
  const char *last12 = HwSerialKey + 8;  // last 12 chars of 20-char serial
  //  memcpy(key + (KEY_LEN - 14), last12, 12);

  for (uint8_t i = 0; i < 6; i++) {
    char hexPair[3] = { last12[i * 2], last12[i * 2 + 1], 0 };
    key[(KEY_LEN - 8) + i] = (uint8_t)strtoul(hexPair, NULL, 16);
  }

  randomSeed(millis());
  uint16_t rnd = random(1, 0xFFFF);
  key[KEY_LEN - 2] = rnd >> 8;
  key[KEY_LEN - 1] = rnd & 0xFF;

  //    //DEBUG_PRINT(F("Key: "));
  //   for (uint8_t i = 0; i < KEY_LEN; i++) {
  //     if (key[i] < 0x10)  //DEBUG_PRINT('0');
  // #ifdef SERIAL_DEBUG
  //    // Serial.print(key[i], HEX);
  // #endif
  //      //DEBUG_PRINT(' ');
  //   }
  //    //DEBUG_PRINTLN();

  // Index hardcoded
  uint8_t idx = random(0, 16);
  //DEBUG_PRINTLN(F("Using idx: "));
  //DEBUG_PRINTLN(idx);
  delay(10);
  //DEBUG_PRINTLN(F("Bf encr"));
  delay(10);
  char encBuf[MAX_MESSAGE_LEN * 2 + 3];

#if ENCRYPTION_ENABLED

  if (!encryptWithIdx(msg, key, HwSerialKey, idx, encBuf, sizeof(encBuf))) {
    //DEBUG_PRINTLN(F("Encrypt failed"));
    return;
  }
  //DEBUG_PRINT(F("Encrypted: "));
  //DEBUG_PRINTLN(encBuf);
#else
  //DEBUG_PRINTLN(F("Encryption disabled"));
  strncpy(encBuf, msg, sizeof(encBuf));
  encBuf[sizeof(encBuf) - 1] = '\0';
#endif

  //DEBUG_PRINTLN(F("After encryption"));

  // ---- Build final TX buffer: idx + rnd + encBuf ----
  char finalBuf[HEX_BUFFER_LEN + 16];  // safety margin
  snprintf(finalBuf, sizeof(finalBuf), "%02X%04X%s", idx, rnd, encBuf);

  // Copy to txBuffer for sending
  strncpy(txBuffer, finalBuf, sizeof(txBuffer));
  txBuffer[sizeof(txBuffer) - 1] = '\0';

  //DEBUG_PRINT(F("TX Payload: ["));
  //DEBUG_PRINTLN(txBuffer);
  //DEBUG_PRINT(F("]"));
  //DEBUG_PRINTLN(F("Sending to LoRa..."));
  if (!bit_is_set(SREG, 7)) {
    //DEBUG_PRINTLN(F("Interrupts are OFF, enabling now"));
    sei();
  }
  send_lora_data((uint8_t *)txBuffer, strlen(txBuffer));
  //DEBUG_PRINTLN(F("LoRa Send Complete"));

#if ENCRYPTION_ENABLED
  // Optional self-test: decrypt
  char decBuf[MAX_MESSAGE_LEN + 1];
  //DEBUG_PRINTLN(F("Trying decrypt..."));
  if (decryptWithIdx(encBuf, key, HwSerialKey, decBuf, sizeof(decBuf))) {
    //DEBUG_PRINT(F("Decrypted: "));
    //DEBUG_PRINTLN(decBuf);
  } else {
    //DEBUG_PRINTLN(F("Decrypt failed"));
  }
#endif

  //DEBUG_PRINTLN(F("--------------------"));
}




static inline void hwbuttonInit() {
  for (int i = 0; i < NUM_BUTTONS; ++i) {
    buttonStates[i] = HIGH;
    lastButtonStates[i] = HIGH;
    lastDebounceTimes[i] = 0;
    //pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

static inline void hwbuttonPinLoop() {
  for (int i = 0; i < NUM_BUTTONS; ++i) {
    int reading = digitalRead(buttonPins[i]);
    if (reading != lastButtonStates[i]) lastDebounceTimes[i] = millis();
    if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;
        if (buttonStates[i] == LOW) {  // pressed
          switch (i) {
            case 0:
              funcM1LGreen();
              sendLoraData("[M1ON]");
              buzBeepNonBlocking(150);
              funcLedReset();
              watchdogReset();
              lowPowerKick();
              break;
            case 1:
              funcM1LRed();
              sendLoraData("[M1OFF]");
              buzBeepNonBlocking(150);
              funcLedReset();
              watchdogReset();
              lowPowerKick();
              break;
            case 2:
              funcM2LRed();
              sendLoraData("[M1OFF]");
              buzBeepNonBlocking(150);
              funcLedReset();
              watchdogReset();
              lowPowerKick();
              break;
            case 3:
              funcStaLBlue();
              sendLoraData("[STB]");
              buzBeepNonBlocking(150);
              funcLedReset();
              watchdogReset();
              lowPowerKick();
              break;
            case 4:
              funcM2LGreen();
              sendLoraData("[M1ON]");
              buzBeepNonBlocking(150);
              funcLedReset();
              watchdogReset();
              lowPowerKick();
              break;
          }
        }
      }
    }
    lastButtonStates[i] = reading;
  }
}
