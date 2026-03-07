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

  // Boot re-pair: hold M1_OFF + STA simultaneously for >= 5 s at power-on.
  // Both pins are INPUT_PULLUP (settled immediately after hwPinInit()).
  // enterRemNodePairMode() is called AFTER sx1268Init() so the radio is ready.
  bool bootRepair = false;
  if (digitalRead(M1_OFF_BTN) == LOW && digitalRead(STA_BTN) == LOW) {
    unsigned long holdStart = millis();
    while (digitalRead(M1_OFF_BTN) == LOW && digitalRead(STA_BTN) == LOW) {
      if (millis() - holdStart >= 5000UL) {
        bootRepair = true;
        break;
      }
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

  // Trigger re-pair now that radio is initialised
  if (bootRepair) {
    clearPeerSerial();
    enterRemNodePairMode();   // switches to PAIR channel, starts beaconing
  }

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
  ackReception();      // STA retry / timeout handler (defined in button.h)
  sx1268Func();
  pairRemNodeTick();   // handles pairing state machine; auto-pairs if no peer stored
  // funcLedReset() removed from here — every LED action already resets at its own end.
  // Removing it allows pairRemNodeTick() 4-phase blink to stay visible between loop ticks.

  if (wdtEnabled) {
    watchdogReset();
  }

  delay(5);
}
