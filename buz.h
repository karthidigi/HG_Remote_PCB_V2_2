
// ---------------- NOTE DEFINITIONS ----------------

#define NOTE_C5 523
#define NOTE_A4 440
#define NOTE_F4 349
#define NOTE_E5 659
#define NOTE_G5 784
#define NOTE_C6 1047
#define NOTE_E6 1319


// ---------------- CUSTOM GAME-LIKE MELODY ----------------
// "Victory Fanfare" - higher pitch version
static const int melody[] PROGMEM = {
  NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6,
  NOTE_E6, NOTE_C6, NOTE_G5, NOTE_C6
};

static const int noteDurations[] PROGMEM = {
  8, 8, 8, 8,
  4, 8, 8, 2
};

void MotorOnTone() {
  size_t length = sizeof(melody) / sizeof(melody[0]);
  for (size_t i = 0; i < length; i++) {
    unsigned long noteDuration = 1000UL / (int)pgm_read_word(&noteDurations[i]);
    int freq = (int)pgm_read_word(&melody[i]);

    unsigned long start = millis();
    unsigned int halfPeriod = 1000000UL / (2 * freq);

    while (millis() - start < noteDuration) {
      digitalWrite(BUZ, LOW);  // buzzer ON (active-low)
      delayMicroseconds(halfPeriod);
      digitalWrite(BUZ, HIGH);  // buzzer OFF
      delayMicroseconds(halfPeriod);
    }

    delay(noteDuration * 0.30);  // Small pause
  }
  digitalWrite(BUZ, HIGH);  // Ensure idle
}

void noNetworkTone() {
  const int no_melody[]    = { NOTE_C5, NOTE_A4, NOTE_F4 };
  const int no_durations[] = { 16, 16, 8 };

  size_t length = sizeof(no_melody) / sizeof(no_melody[0]);
  for (size_t i = 0; i < length; i++) {
    unsigned long noteDuration = 1000UL / no_durations[i];
    int freq = no_melody[i];

    unsigned long start = millis();
    unsigned int halfPeriod = 1000000UL / (2 * freq);

    while (millis() - start < noteDuration) {
      digitalWrite(BUZ, LOW);  // buzzer ON (active-low)
      delayMicroseconds(halfPeriod);
      digitalWrite(BUZ, HIGH);  // buzzer OFF
      delayMicroseconds(halfPeriod);
    }

    delay(noteDuration * 0.2);  // Tiny pause
  }
  digitalWrite(BUZ, HIGH);  // Ensure idle
}

//Buzzer: short blocking beep only for small durations
static inline void buzBeep(uint16_t ms) {
  digitalWrite(BUZ, LOW);
  delay(ms);
  digitalWrite(BUZ, HIGH);
}
