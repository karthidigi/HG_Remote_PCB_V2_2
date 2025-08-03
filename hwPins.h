#define M1_ON_BTN  PIN_PB5
#define M1_OFF_BTN PIN_PC3

#define M2_ON_BTN PIN_PB2
#define M2_OFF_BTN PIN_PB1

#define STA_BTN PIN_PC2

#define LED_ANODE PIN_PC0
#define LED_J1 PIN_PC1
#define LED_J2 PIN_PB4
#define LED_J3 PIN_PB3

#define BUZ PIN_PB0

#define PIN_MOSI   PIN_PA1
#define PIN_MISO   PIN_PA2
#define PIN_SCK    PIN_PA3
#define PIN_NSS    PIN_PA4
#define PIN_BUSY   PIN_PA5
#define PIN_NRST   PIN_PA6
#define PIN_DIO1   PIN_PA7

//////////////////////////////////////////////////
//////////////////////////////////////////////////

void hwPinInit() {
  pinMode(BUZ, OUTPUT);

  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  pinMode(LED_J3, OUTPUT);

  pinMode(M1_ON_BTN, INPUT_PULLUP);
  pinMode(M1_OFF_BTN, INPUT_PULLUP);
  pinMode(M2_ON_BTN, INPUT_PULLUP);
  pinMode(M2_OFF_BTN, INPUT_PULLUP);
  pinMode(STA_BTN, INPUT_PULLUP);

  digitalWrite(LED_ANODE, LOW);
  digitalWrite(LED_J1, LOW);
  digitalWrite(LED_J2, LOW);
  digitalWrite(LED_J3, LOW);
  digitalWrite(BUZ, HIGH);

}

//////////////////////////////////////////////////
