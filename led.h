/////////////////////////////////////
// LED_ANODE = PIN_PC0 → PORTC bit 0 (PIN0_bm)
// LED_J1    = PIN_PC1 → PORTC bit 1 (PIN1_bm)
// LED_J2    = PIN_PB4 → PORTB bit 4 (PIN4_bm)
// LED_J3    = PIN_PB3 → PORTB bit 3 (PIN3_bm)

void funcLedReset() {
  PORTC.DIRCLR = PIN0_bm | PIN1_bm;  // ANODE, J1 → input (high-Z)
  PORTB.DIRCLR = PIN4_bm | PIN3_bm;  // J2, J3   → input (high-Z)
}

void funcStaLWhite() {
  PORTC.DIRCLR = PIN0_bm | PIN1_bm;
  PORTB.DIRCLR = PIN4_bm | PIN3_bm;
  PORTC.OUTSET = PIN0_bm;            // ANODE HIGH
  PORTC.OUTCLR = PIN1_bm;            // J1 LOW
  PORTB.OUTCLR = PIN4_bm | PIN3_bm;  // J2, J3 LOW
  PORTC.DIRSET = PIN0_bm | PIN1_bm;
  PORTB.DIRSET = PIN4_bm | PIN3_bm;
}

void funcStaLBlue() {
  PORTC.DIRCLR = PIN0_bm | PIN1_bm;
  PORTB.DIRCLR = PIN4_bm | PIN3_bm;
  PORTC.OUTSET = PIN0_bm;  // ANODE HIGH
  PORTB.OUTCLR = PIN3_bm;  // J3 LOW
  PORTC.DIRSET = PIN0_bm;
  PORTB.DIRSET = PIN3_bm;
}

void funcStaLRed() {
  PORTC.DIRCLR = PIN0_bm | PIN1_bm;
  PORTB.DIRCLR = PIN4_bm | PIN3_bm;
  PORTB.OUTSET = PIN3_bm;            // J3 HIGH
  PORTC.OUTSET = PIN0_bm;            // ANODE HIGH
  PORTC.OUTCLR = PIN1_bm;            // J1 LOW
  PORTC.DIRSET = PIN0_bm | PIN1_bm;
  PORTB.DIRSET = PIN3_bm;
}

void funcM1LRed() {
  PORTC.DIRCLR = PIN0_bm | PIN1_bm;
  PORTB.DIRCLR = PIN4_bm | PIN3_bm;
  PORTC.OUTSET = PIN0_bm;  // ANODE HIGH
  PORTC.OUTCLR = PIN1_bm;  // J1 LOW
  PORTC.DIRSET = PIN0_bm | PIN1_bm;
}

void funcM1LGreen() {
  PORTC.DIRCLR = PIN0_bm | PIN1_bm;
  PORTB.DIRCLR = PIN4_bm | PIN3_bm;
  PORTC.OUTSET = PIN0_bm;  // ANODE HIGH
  PORTB.OUTCLR = PIN4_bm;  // J2 LOW
  PORTC.DIRSET = PIN0_bm;
  PORTB.DIRSET = PIN4_bm;
}

void funcM1Yellow() {
  PORTC.DIRCLR = PIN0_bm | PIN1_bm;
  PORTB.DIRCLR = PIN4_bm | PIN3_bm;
  PORTC.OUTSET = PIN0_bm;  // ANODE HIGH
  PORTC.OUTCLR = PIN1_bm;  // J1 LOW
  PORTB.OUTCLR = PIN4_bm;  // J2 LOW
  PORTC.DIRSET = PIN0_bm | PIN1_bm;
  PORTB.DIRSET = PIN4_bm;
}

void funcM2LGreen() {
  PORTC.DIRCLR = PIN0_bm | PIN1_bm;
  PORTB.DIRCLR = PIN4_bm | PIN3_bm;
  PORTB.OUTSET = PIN4_bm;  // J2 HIGH
  PORTB.DIRSET = PIN4_bm;
}

void funcM2LRed() {
  PORTC.DIRCLR = PIN0_bm | PIN1_bm;
  PORTB.DIRCLR = PIN4_bm | PIN3_bm;
  PORTB.OUTSET = PIN3_bm;  // J3 HIGH
  PORTB.DIRSET = PIN3_bm;
}

void funcM2Yellow() {
  PORTC.DIRCLR = PIN0_bm | PIN1_bm;
  PORTB.DIRCLR = PIN4_bm | PIN3_bm;
  PORTB.OUTSET = PIN4_bm | PIN3_bm;  // J2, J3 HIGH
  PORTB.DIRSET = PIN4_bm | PIN3_bm;
}

void funcLedTest() {
  funcStaLWhite();  delay(50);
  funcStaLBlue();   delay(50);
  funcM1LRed();     delay(50);
  funcM1LGreen();   delay(50);
  funcM1Yellow();   delay(50);
  funcM2LRed();     delay(50);
  funcM2LGreen();   delay(50);
  funcM2Yellow();   delay(50);
  funcLedReset();
}

void lowBattAlert() {
  funcM1LRed();  delay(500);
  funcLedReset(); delay(500);
  funcM1LRed();
}
