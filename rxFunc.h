///////////////////////////////////////
char encapData[MAX_MESSAGE_LEN];
///////////////////////////////////////
void rxFunc() {
  msgTxd = 0;
  if (strcmp(encapData, "1N") == 0) {
    //DEBUG_PRINTN(F("M11 received"));
    buttonEn[0] = DISABLED;
    buttonEn[1] = ENABLED;
    buttonEn[2] = DISABLED;
    buttonEn[3] = DISABLED;
    funcM1LGreen();
    MotorOnTone();
    funcLedReset();

  } else if (strcmp(encapData, "1F") == 0) {
    //DEBUG_PRINTN(F("M10 received"));
    buttonEn[0] = ENABLED;  
    buttonEn[1] = DISABLED;
    buttonEn[2] = ENABLED;
    buttonEn[3] = DISABLED;
    funcM1LRed();
    buzBeep(BUZZ_OFF);
    delay(1000);
    funcLedReset();
  } else if (strcmp(encapData, "2N") == 0) {
    //DEBUG_PRINTN(F("M21 received"));
    buttonEn[0] = DISABLED;
    buttonEn[1] = DISABLED;
    buttonEn[2] = ENABLED;
    buttonEn[3] = DISABLED;
    funcM2LGreen();
    buzBeep(BUZZ_OFF);
    delay(1000);
    funcLedReset();
  } else if (strcmp(encapData, "2F") == 0) {
    //DEBUG_PRINTN(F("M20 received"));
    buttonEn[0] = ENABLED;  //
    buttonEn[1] = DISABLED;  //
    buttonEn[2] = ENABLED;  //
    buttonEn[3] = DISABLED;
    funcM2LRed();
    buzBeep(BUZZ_OFF);
    delay(1000);
    funcLedReset();
  }else if (strcmp(encapData, "A0") == 0) {
    //DEBUG_PRINTN(F("M20 received"));
    buttonEn[0] = ENABLED;  //
    buttonEn[1] = DISABLED;  //
    buttonEn[2] = ENABLED;  //
    buttonEn[3] = DISABLED;
    funcM2LRed();
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
