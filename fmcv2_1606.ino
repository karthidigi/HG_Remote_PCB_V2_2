#include "hwPins.h"
#include "buz.h"
#include "led.h"
#include "loraCom.h"
#include "button.h"

void setup() {

  //Serial.begin(9600);
  hwPinInit();
  funcLedTest();
  buzBeep(300);
  loraInit();

}

void loop() {
  hwbuttonPin();
  loraStbTx();
  buzStbAlert();
}
