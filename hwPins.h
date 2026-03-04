#pragma once
/////////////////////////////////////////
// Buttons
#define M1_ON_BTN PIN_PB5
#define M1_OFF_BTN PIN_PC3
#define M2_ON_BTN PIN_PB2
#define M2_OFF_BTN PIN_PB1
#define STA_BTN PIN_PC2

// LEDs / buzzer
#define LED_ANODE PIN_PC0
#define LED_J1 PIN_PC1
#define LED_J2 PIN_PB4
#define LED_J3 PIN_PB3
#define BUZ PIN_PB0

// LoRa mapping (kept for compatibility)
#define LLCC68_NSS PIN_PA4
#define LLCC68_SCK PIN_PA3
#define LLCC68_MOSI PIN_PA1
#define LLCC68_MISO PIN_PA2
#define LLCC68_RESET PIN_PA6
#define LLCC68_BUSY PIN_PA5
#define LLCC68_DIO1 PIN_PA7

// SX1268 driver aliases — same physical pins as LLCC68 (pin-compatible modules)
#define SX1268_NSS LLCC68_NSS
#define SX1268_BUSY LLCC68_BUSY
#define SX1268_RESET_PIN LLCC68_RESET
#define SX1268_DIO1 LLCC68_DIO1

////////////////////////////////////////
// Initialize hardware pins

void hwPinInit() {
  pinMode(BUZ, OUTPUT);
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  pinMode(LED_J3, OUTPUT);

  pinMode(M1_ON_BTN, INPUT_PULLUP);
  pinMode(M1_OFF_BTN, INPUT_PULLUP);
  //pinMode(M2_ON_BTN, INPUT_PULLUP);  //TX so need to comment for debug
  pinMode(M2_OFF_BTN, INPUT_PULLUP);
  pinMode(STA_BTN, INPUT_PULLUP);

  digitalWrite(LED_ANODE, LOW);
  digitalWrite(LED_J1, LOW);
  digitalWrite(LED_J2, LOW);
  digitalWrite(LED_J3, LOW);
  digitalWrite(BUZ, HIGH);
}
