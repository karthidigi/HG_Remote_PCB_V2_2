
///////////////////////////////////////
char encapData[MAX_MESSAGE_LEN];
///////////////////////////////////////
void rxFunc() {
  msgTxd = 0;
  if (strcmp(encapData, "1N") == 0) {
    //DEBUG_PRINTN(F("M11 received"));
    buttonEn[0] = 0;
    buttonEn[1] = 1;
    //buttonEn[2]=0;
    //buttonEn[3]=0;
    funcM1LGreen();
    MotorOnTone();
    funcLedReset();

  } else if (strcmp(encapData, "1F") == 0) {
    //DEBUG_PRINTN(F("M10 received"));
    buttonEn[0] = 1;
    buttonEn[1] = 0;
    //buttonEn[2]=1;
    //buttonEn[3]=0;
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
