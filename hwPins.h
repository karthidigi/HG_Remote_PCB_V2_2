#pragma once
///////////////////////////////////////////
// Buttons — v2-2 board: 3 buttons only
#define M1_ON_BTN  PIN_PB5
#define M1_OFF_BTN PIN_PC3
// PIN_PB2 (was M2_ON_BTN)  — not fitted on v2-2 board
// PIN_PB1 (was M2_OFF_BTN) — open drain, unused on v2-2
#define STA_BTN    PIN_PC2

// LEDs — v2-2: cathode-sink topology (drive LOW = on, DIRCLR = off)
// PB3 = blue LED cathode, PC1 = red LED cathode, PB4 = green LED cathode
#define LED_BLUE   PIN_PB3
#define LED_RED    PIN_PC1
#define LED_GREEN  PIN_PB4
#define BUZ        PIN_PB0

// LoRa mapping (kept for compatibility)
#define LLCC68_NSS   PIN_PA4
#define LLCC68_SCK   PIN_PA3
#define LLCC68_MOSI  PIN_PA1
#define LLCC68_MISO  PIN_PA2
#define LLCC68_RESET PIN_PA6
#define LLCC68_BUSY  PIN_PA5
#define LLCC68_DIO1  PIN_PA7

// SX1268 driver aliases — same physical pins as LLCC68 (pin-compatible modules)
#define SX1268_NSS       LLCC68_NSS
#define SX1268_BUSY      LLCC68_BUSY
#define SX1268_RESET_PIN LLCC68_RESET
#define SX1268_DIO1      LLCC68_DIO1

////////////////////////////////////////
// Initialize hardware pins

void hwPinInit() {
  pinMode(BUZ, OUTPUT);

  // All LED pins start as INPUT (high-Z) — LEDs off
  pinMode(LED_BLUE,  INPUT);   // PB3 = blue cathode
  pinMode(LED_RED,   INPUT);   // PC1 = red cathode
  pinMode(LED_GREEN, INPUT);   // PB4 = green cathode

  pinMode(M1_ON_BTN,  INPUT_PULLUP);
  pinMode(M1_OFF_BTN, INPUT_PULLUP);

  // PB1 (was M2_OFF_BTN): open drain on v2-2 — plain INPUT, no pullup
  PORTB.DIRCLR   = PIN1_bm;
  PORTB.PIN1CTRL = 0;        // no pullup, no interrupt

  // PB2 (was M2_ON_BTN, not fitted): leave as reset default

  pinMode(STA_BTN, INPUT_PULLUP);

  digitalWrite(BUZ, HIGH);   // active-low buzzer idle
}
