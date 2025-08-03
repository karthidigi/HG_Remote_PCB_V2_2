#include "hwPins.h"
#include "led.h"
#include "buz.h"
#include "loraCom.h"
#include "button.h"

void setup() {

  hwPinInit();
  funcLedTest();
  buzBeep(300);
  loraInit();

}

void loop() {
  hwbuttonPin();
  //loraStbTx();
  //buzStbAlert();
  loraRxFunc();
}
