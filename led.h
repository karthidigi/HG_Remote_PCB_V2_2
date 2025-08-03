void funcLedReset(){
  pinMode(LED_J1, INPUT);
  pinMode(LED_J2, INPUT);
  pinMode(LED_J3, INPUT);
  pinMode(LED_ANODE, INPUT);
}


void funcStaWhite(){
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

void funcStaLBlue(){
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J3, LOW); 
}

void funcM1LRed(){
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J1, LOW);
}

void funcM1LGreen(){
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J2, LOW);
}

void funcM1Yellow(){
  funcLedReset();
  pinMode(LED_ANODE, OUTPUT);
  pinMode(LED_J1, OUTPUT);
  pinMode(LED_J2, OUTPUT);
  digitalWrite(LED_ANODE, HIGH);
  digitalWrite(LED_J1, LOW);
  digitalWrite(LED_J2, LOW);
}

void funcM2LRed(){
  funcLedReset();
  pinMode(LED_J2, OUTPUT);
  digitalWrite(LED_J2, HIGH);
}

void funcM2LGreen(){ 
  funcLedReset();
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_J3, HIGH);
}

void funcM2Yellow(){
  funcLedReset();
  pinMode(LED_J2, OUTPUT);
  pinMode(LED_J3, OUTPUT);
  digitalWrite(LED_J3, HIGH);
  digitalWrite(LED_J2, HIGH);
}

void funcLedTest(){
  funcStaWhite(); delay(300);
  funcStaLBlue(); delay(300);
  funcM1LRed(); delay(300);
  funcM1LGreen(); delay(300);
  funcM1Yellow(); delay(300);
  funcM2LRed(); delay(300);
  funcM2LGreen(); delay(300);
  funcM2Yellow(); delay(300);
  funcLedReset();
}
