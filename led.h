/////////////////////////////////////

void funcLedReset() {
  pinMode(LED_J1, INPUT);
  pinMode(LED_J2, INPUT);
  pinMode(LED_J3, INPUT);
  pinMode(LED_ANODE, INPUT);
}


void funcStaLWhite() {
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J1, LOW);
  digitalWrite(LED_J2, LOW);
  digitalWrite(LED_J3, LOW);
}

void funcStaLBlue() {
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J3, LOW);
}

void funcStaLRed() {
  funcLedReset();
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_J3, HIGH);
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J1, LOW);
}

void funcM1LRed() {
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J1, LOW);
}

void funcM1LGreen() {
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J2, LOW);
}

void funcM1Yellow() {
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J1, LOW);
  digitalWrite(LED_J2, LOW);
}

void funcM2LGreen() {
  funcLedReset();
  pinMode(LED_J2, OUTPUT);
  digitalWrite(LED_J2, HIGH);
}

void funcM2LRed() {
  funcLedReset();
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_J3, HIGH);
}

void funcM2Yellow() {
  funcLedReset();
  pinMode(LED_J2, OUTPUT);
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_J3, HIGH);
  digitalWrite(LED_J2, HIGH);
}

void funcLedTest() {
  funcStaLWhite();
  delay(50);
  funcStaLBlue();
  delay(50);
  funcM1LRed();
  delay(50);
  funcM1LGreen();
  delay(50);
  funcM1Yellow();
  delay(50);
  funcM2LRed();
  delay(50);
  funcM2LGreen();
  delay(50);
  funcM2Yellow();
  delay(50);
  funcLedReset();
}

void lowBattAlert() {
  funcM1LRed();
  delay(500);
  funcLedReset();
  delay(500);
  funcM1LRed();
}