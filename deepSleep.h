#include <avr/sleep.h>    // For deep sleep
#include <avr/wdt.h>      // For watchdog timer (optional error recovery)
#include <avr/cpufunc.h>  // For _PROTECTED_WRITE (ATtiny 0/1-series)

// Pin definitions
#define INTERRUPT_PIN PIN_PA7  // PA7: Interrupt pin (e.g., LLCC68 DIO1)
#define LED_PIN       PIN_PA0  // PA0: Optional LED for status (uncomment if used)

// Timing constants
const unsigned long INACTIVITY_TIMEOUT_MS = 30000; // 30 seconds to enter sleep

// Global variables
volatile bool interrupt_triggered = false;
unsigned long last_activity_time = 0;

// Interrupt service routine for PA7
void interrupt_isr() {
  interrupt_triggered = true;
}

// Error handling function (optional WDT reset)
void handleError(const char* errorMsg) {
  Serial.print("Error: ");
  Serial.println(errorMsg);
  
  // Optional: Blink LED on PA0 (uncomment and wire LED + resistor)
  // pinMode(LED_PIN, OUTPUT);
  // while(1) {
  //   digitalWrite(LED_PIN, HIGH); delay(200);
  //   digitalWrite(LED_PIN, LOW); delay(200);
  // }
  
  // Optional: Enable WDT for auto-reset after ~2s (max for ATtiny1606)
  #ifdef __AVR_ATtiny1606__
  wdt_enable(WDTO_2S);  // ~2s timeout
  #else
  _PROTECTED_WRITE(WDT.CTRLA, WDT_PERIOD_2KCLK_gc);  // Fallback: direct 2s WDT
  #endif
  
  while(1) {
    // wdt_reset();  // Uncomment to prevent WDT reset (for debugging)
    delay(1);
  }
}

// Enter deep sleep (Power-Down mode)
void enterDeepSleep() {
  Serial.println("Entering deep sleep...");
  Serial.flush();  // Ensure all serial data is sent
  Serial.end();    // Disable Serial to save power

  // Disable ADC to reduce power
  ADC0.CTRLA &= ~ADC_ENABLE_bm;

  // Configure all pins to minimize leakage
  // Outputs low, inputs with pull-ups or floating
  PORTA.DIRSET = _BV(0) | _BV(4) | _BV(6); // PA0, PA4, PA6 as outputs
  PORTA.OUTCLR = _BV(0) | _BV(4) | _BV(6); // Set low: PA0 (LED), PA4 (NSS), PA6 (RESET)
  PORTA.DIRCLR = _BV(1) | _BV(2) | _BV(3) | _BV(5) | _BV(7); // PA1,2,3,5,7 as inputs
  PORTA.PIN1CTRL |= PORT_PULLUPEN_bm; // PA1 (MOSI) pull-up
  PORTA.PIN2CTRL |= PORT_PULLUPEN_bm; // PA2 (MISO) pull-up
  PORTA.PIN3CTRL |= PORT_PULLUPEN_bm; // PA3 (SCK) pull-up
  PORTA.PIN5CTRL |= PORT_PULLUPEN_bm; // PA5 (BUSY) pull-up
  PORTA.PIN7CTRL |= PORT_PULLUPEN_bm; // PA7 (interrupt) pull-up (or use external pull-down)

  // Set sleep mode to Power-Down
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  // Ensure interrupt is enabled
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interrupt_isr, RISING);

  // Enter sleep
  sleep_cpu();

  // On wakeup (after interrupt), execution resumes here
  sleep_disable();
  
  // Reinitialize Serial for debugging
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Woke up from deep sleep!");
  
  // Reinitialize ADC if needed (disabled earlier)
  ADC0.CTRLA |= ADC_ENABLE_bm;

  // Reset activity timer
  last_activity_time = millis();
  interrupt_triggered = false;
}

// void setup() {
//   // Initialize Serial (PB2:TX, PB3:RX)
//   Serial.begin(9600);
//   while (!Serial);
//   Serial.println("ATtiny1606 Deep Sleep Start");

//   // Initialize pins
//   pinMode(INTERRUPT_PIN, INPUT); // PA7 as input (interrupt pin)
//   // Optional: LED pin setup
//   // pinMode(LED_PIN, OUTPUT);
//   // digitalWrite(LED_PIN, LOW);

//   // Attach interrupt for PA7 (rising edge)
//   attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interrupt_isr, RISING);

//   // Initialize activity timer
//   last_activity_time = millis();
// }

// void loop() {
//   // Check for interrupt
//   if (interrupt_triggered) {
//     Serial.println("Interrupt on PA7!");
//     last_activity_time = millis();
//     interrupt_triggered = false;
//     // Optional: Toggle LED
//     // digitalWrite(LED_PIN, HIGH); delay(50); digitalWrite(LED_PIN, LOW);
//   }

//   // Check for inactivity timeout
//   if (millis() - last_activity_time >= INACTIVITY_TIMEOUT_MS) {
//     enterDeepSleep();
//   }

//   // Optional: Feed WDT if enabled globally
//   // wdt_reset();
// }