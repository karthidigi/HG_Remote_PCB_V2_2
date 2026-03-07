///////////////////////////////////////
char encapData[MAX_MESSAGE_LEN];
///////////////////////////////////////
void rxFunc() {
  msgTxd = 0;
  if (strcmp(encapData, "1N") == 0) {
    // Motor 1 ON confirmed
    buttonEn[0] = DISABLED;  // M1_ON  (motor running — can't turn on again)
    buttonEn[1] = ENABLED;   // M1_OFF
    buttonEn[2] = ENABLED;   // STA
    funcM1LGreen();
    MotorOnTone();
    funcLedReset();

  } else if (strcmp(encapData, "1F") == 0) {
    // Motor 1 OFF confirmed
    buttonEn[0] = ENABLED;   // M1_ON
    buttonEn[1] = DISABLED;  // M1_OFF (motor stopped — can't turn off again)
    buttonEn[2] = ENABLED;   // STA
    funcM1LRed();
    buzBeep(BUZZ_OFF);
    delay(1000);
    funcLedReset();

  } else if (strcmp(encapData, "A0") == 0) {
    // Auto-stop / alarm: treat as motor OFF state
    buttonEn[0] = ENABLED;   // M1_ON
    buttonEn[1] = DISABLED;  // M1_OFF
    buttonEn[2] = ENABLED;   // STA
    funcM1LRed();
    buzBeep(BUZZ_OFF);
    delay(1000);
    funcLedReset();

  } else {
    delay(1000);
    funcM1Yellow();
    delay(200);
    funcLedReset();
  }
}
