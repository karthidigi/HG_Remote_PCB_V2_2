#pragma once
#define ENABLED 1
#define DISABLED 0
////////////////////////////////////////
// 3 buttons: M1_ON(0), M1_OFF(1), STA(2)
uint8_t buttonEn[3] = { DISABLED, DISABLED, ENABLED };
bool msgTxd = 0;
