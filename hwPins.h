// ---------- FILE: hwPins.h ----------
#pragma once

// Buttons
#define M1_ON_BTN  PIN_PB5
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

// Initialize hardware pins
static inline void hwPinInit() {
  pinMode(BUZ, OUTPUT);
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  pinMode(LED_J3, OUTPUT);

  pinMode(M1_ON_BTN, INPUT_PULLUP);
  pinMode(M1_OFF_BTN, INPUT_PULLUP);
  pinMode(M2_ON_BTN, INPUT_PULLUP); //TX so need to comment for debug
  pinMode(M2_OFF_BTN, INPUT_PULLUP);
  pinMode(STA_BTN, INPUT_PULLUP);

  digitalWrite(LED_ANODE, LOW);
  digitalWrite(LED_J1, LOW);
  digitalWrite(LED_J2, LOW);
  digitalWrite(LED_J3, LOW);
  digitalWrite(BUZ, HIGH);
}

// LED utilities
static inline void funcLedReset(){
  pinMode(LED_J1, INPUT);
  pinMode(LED_J2, INPUT);
  pinMode(LED_J3, INPUT);
  pinMode(LED_ANODE, INPUT);
}



void funcStaWhite(){
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J1, LOW);
  digitalWrite(LED_J2, LOW);
  digitalWrite(LED_J3, LOW);
}

void funcStaLBlue(){
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J3, LOW); 
}

void funcM1LRed(){
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J1, LOW);
}

void funcM1LGreen(){
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J2, LOW);
}


void funcM1Yellow(){
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J1, LOW);
  digitalWrite(LED_J2, LOW);
}

void funcM2LGreen(){
  funcLedReset();
  pinMode(LED_J2, OUTPUT);
  digitalWrite(LED_J2, HIGH);
}

void funcMLRed(){
  funcLedReset();
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_J3, HIGH);
   pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J1, LOW);
}
  

void funcM2LRed(){ 
  funcLedReset();
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_J3, HIGH);
}

void funcM2Yellow(){
  funcLedReset();
  pinMode(LED_J2, OUTPUT);
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_J3, HIGH);
  digitalWrite(LED_J2, HIGH);
}

void funcLedTest(){
  funcStaWhite(); delay(100);
  funcStaLBlue(); delay(100);
  funcM1LRed(); delay(100);
  funcM1LGreen(); delay(100);
  funcM1Yellow(); delay(100);
  funcM2LRed(); delay(100);
  funcM2LGreen(); delay(100);
  funcM2Yellow(); delay(100);
  funcLedReset();
}