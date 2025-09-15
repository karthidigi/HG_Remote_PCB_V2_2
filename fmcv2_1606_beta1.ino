#include "zSettings.h"
#include "hwPins.h"
#include "led.h"
#include "buz.h"
#include "debug.h"
#include "watDog.h"
#include "eeprom.h"
#include "aesMain.h"
#include "devId.h"
#include "encrypt.h"
#include "rxFunc.h"
#include "decrypt.h"
#include "llcc68Main.h"
#include "lowPow.h"
#include "button.h"

void setup() {
  hwPinInit();
  funcLedTest();
  buzBeep(100);
  hwSerialInit();
  getDeviceSerId();
  aesInit("[horizon]");
  llcc68Init();
  hwbuttonInit();
  lowPowerInit();
  if (!wdtEnabled) {
    watchdogInit();
    wdtEnabled = true;
  }
  ///////////////////////
  savePeerSerial("42407197000090220136");
  delay(100);
 
}

void loop() {
  hwbuttonFunc();
  llcc68Func();
  lowPowerPoll();
  if (wdtEnabled) {
    watchdogReset();
  }
  delay(5);
}