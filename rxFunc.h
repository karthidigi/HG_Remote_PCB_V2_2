
///////////////////////////////////////
char encapData[MAX_MESSAGE_LEN]; 
///////////////////////////////////////
void rxFunc() {
  if (strcmp(encapData, "M11") == 0) {
    //DEBUG_PRINTN(F("M11 received"));
    funcM1LGreen();
    buzBeep(150);
    delay(200);
    buzBeep(150);

  } else if (strcmp(encapData, "M21") == 0) {
    //DEBUG_PRINTN(F("M21 received"));
    funcM2LGreen();
    buzBeep(150);
    delay(200);
    buzBeep(150);
  } else if (strcmp(encapData, "M10") == 0) {
    //DEBUG_PRINTN(F("M10 received"));
    funcM1LRed();
    buzBeep(500);
  } else if (strcmp(encapData, "M20") == 0) {
    //DEBUG_PRINTN(F("M20 received"));
    funcM2LRed();
    buzBeep(500);
  } else if (strcmp(encapData, "M00") == 0) {
    //DEBUG_PRINTN(F("M00 received"));
    funcStaLRed();
    buzBeep(500);
  } else {
    funcM1Yellow();
    delay(200);
  }
  funcLedReset();
}
