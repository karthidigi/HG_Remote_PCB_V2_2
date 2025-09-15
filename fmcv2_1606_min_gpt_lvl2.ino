#include "zSettings.h"

#include "hwPins.h"
#include "debug.h"
#include "eeprom.h"
#include "aesMain.h"
#include "llcc68Main.h"
#include "utils.h"
#include "button.h"

bool wdtEnabled = false;

void setup() {
  savePeerSerial("42407197000090220136");
  hwPinInit();

// Serial debug
#ifdef SERIAL_DEBUG
  hwSerialInit();  // from debug.h - begins Serial
  //DEBUG_PRINTN(F("Firmware: " FIRMWARE_VERSION " HW: " HARDWARE_VERSION));
#endif

  llcc68_init();
  hwbuttonInit();
  funcLedTest();
  lowPowerInit();
  aesENDE_TEST("[HORIZON]");
  if (!wdtEnabled) {
    watchdogInit();
    wdtEnabled = true;
  }

  delay(100);
  char hwSerialKey[21];
  getChipSerial(hwSerialKey, sizeof(hwSerialKey));  // EEPROM peer serial
  //DEBUG_PRINT("hwSerialKey: ");
  //DEBUG_PRINTLN(hwSerialKey);
}

void loop() {
  hwbuttonPinLoop();
  llcc68_func();
  lowPowerPoll();
   if (wdtEnabled) {
    watchdogReset();
  }
  delay(5);
}
