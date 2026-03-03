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
#include "sx1268Main.h"      // SX1268 radio driver (replaces llcc68Main.h)
#include "lowPow.h"
#include "button.h"
#include "pairRemoteNode.h"  // Starter↔Remote LoRa pairing (must be after sx1268Main.h)

void setup() {
  hwPinInit();

  // Boot re-pair: hold STA for 3 s at power-on to force re-pair.
  // Clears EEPROM peer serial; pairRemNodeTick() will auto-enter pair mode.
  {
    unsigned long holdStart = millis();
    bool doRepair = false;
    while (digitalRead(STA_BTN) == LOW) {
      if (millis() - holdStart >= 3000UL) {
        doRepair = true;
        break;
      }
    }
    if (doRepair) {
      clearPeerSerial();
      funcStaLRed();
      delay(300);
      funcLedReset();
    }
  }

  /////////////////////
  if (battCheck()) {
    funcStaLWhite();
  } else {
    lowBattAlert();
  }
  delay(500);
  funcLedReset();
  ////////////////////
  buzBeep(BUZZ_NOR);
  hwSerialInit();
  getDeviceSerId();
  aesInit("[horizon]");
  sx1268Init();
  lowPowerInit();
  if (!wdtEnabled) {
    watchdogInit();
    wdtEnabled = true;
  }
  // Peer serial (Starter ID) is stored via LoRa pairing — no hardcoded ID.
  // If EEPROM is empty, pairRemNodeTick() auto-enters pairing mode.
}

void loop() {

  lowPowerPoll();
  hwbuttonFunc();
  sx1268Func();
  pairRemNodeTick();   // handles pairing state machine; auto-pairs if no peer stored
  funcLedReset();

  if (wdtEnabled) {
    watchdogReset();
  }

  delay(5);
}
