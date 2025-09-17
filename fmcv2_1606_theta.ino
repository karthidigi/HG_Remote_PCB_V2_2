#include "zSettings.h"
#include "hwPins.h"
#include "led.h"
#include "batVol.h"
#include "buz.h"
#include "debug.h"
#include "states.h"
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
  /////////////////////
  if (battCheck()) {
    funcStaLWhite();
  } else {
    lowBattAlert();
  }
  delay(500);
  //noNetworkTone();
  funcLedReset();
  ////////////////////
  buzBeep(BUZZ_NOR);
  hwSerialInit();
  getDeviceSerId();
  aesInit("[horizon]");
  llcc68Init();
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
  lowPowerPoll();
  hwbuttonFunc();
  llcc68Func();
  if (wdtEnabled) {
    watchdogReset();
  }
  funcLedReset();
  ackReception();
  delay(5);
}