#include "hwPins.h"
#include "led.h"
#include "buz.h"
#include "aesMain.h"
#include "llcc68Main.h"
#include "button.h"

void setup() {
  Serial.begin(115200);
  hwPinInit();
  funcLedTest();
  buzBeep(300);
  llcc68_init();
}

void loop() {
  hwbuttonPin();
  llcc68_func();
}
