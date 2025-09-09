#include <SPI.h>
#include <avr/wdt.h>      // For watchdog timer (use WDTO_2S for ATtiny1606)
#include <avr/cpufunc.h>  // For _PROTECTED_WRITE (ATtiny 0/1-series)
#include "src/llcc68.h"
#include "src/llcc68_hal.h"

// Pin definitions for ATtiny1606
#define LLCC68_NSS PIN_PA4    // PA4: SPI chip select (active low)
#define LLCC68_SCK PIN_PA3    // PA3: SPI clock
#define LLCC68_MOSI PIN_PA1   // PA1: SPI MOSI
#define LLCC68_MISO PIN_PA2   // PA2: SPI MISO
#define LLCC68_RESET PIN_PA6  // PA6: Reset pin (active low)
#define LLCC68_BUSY PIN_PA5   // PA5: Busy pin
#define LLCC68_DIO1 PIN_PA7   // PA7: Interrupt pin (DIO1)

// Optional: Error LED on PA0 (uncomment and wire LED + resistor to PA0)
// #define ERROR_LED_PIN PIN_PA0

// Context structure for LLCC68
typedef struct {
  uint8_t dummy;  // Placeholder for context
} llcc68_context_t;

llcc68_context_t llcc68_context;

// Flag to stop transmission (set to true to abort TX/RX cycle)
volatile bool stop_transmission = false;

// Interrupt handler for DIO1
volatile bool dio1_triggered = false;
void dio1_isr() {
  dio1_triggered = true;
}

// State machine states
enum RadioState {
  STATE_IDLE,
  STATE_TX_SETUP,
  STATE_TX_WAIT,
  STATE_RX_SETUP,
  STATE_RX_WAIT
};

// Global variables for state machine
RadioState radio_state = STATE_IDLE;
unsigned long state_start_time = 0;
const unsigned long TX_TIMEOUT_MS = 5000;        // 5s timeout for TX
const unsigned long RX_TIMEOUT_MS = 5000;        // 5s timeout for RX
const unsigned long IDLE_DURATION_MS = 1000;     // 1s idle between cycles
uint8_t payload[32] = "Hello from ATtiny1606!";  // Default payload
uint8_t buffer[32];                              // Buffer for TX/RX data

// HAL function implementations
llcc68_hal_status_t llcc68_hal_write(const void* context, const uint8_t* command, const uint16_t command_length,
                                     const uint8_t* data, const uint16_t data_length) {
  digitalWrite(LLCC68_NSS, LOW);
  for (uint16_t i = 0; i < command_length; i++) {
    SPI.transfer(command[i]);
  }
  for (uint16_t i = 0; i < data_length; i++) {
    SPI.transfer(data[i]);
  }
  digitalWrite(LLCC68_NSS, HIGH);
  return LLCC68_HAL_STATUS_OK;
}

llcc68_hal_status_t llcc68_hal_read(const void* context, const uint8_t* command, const uint16_t command_length,
                                    uint8_t* data, const uint16_t data_length) {
  digitalWrite(LLCC68_NSS, LOW);
  for (uint16_t i = 0; i < command_length; i++) {
    SPI.transfer(command[i]);
  }
  for (uint16_t i = 0; i < data_length; i++) {
    data[i] = SPI.transfer(0x00);
  }
  digitalWrite(LLCC68_NSS, HIGH);
  return LLCC68_HAL_STATUS_OK;
}

llcc68_hal_status_t llcc68_hal_reset(const void* context) {
  digitalWrite(LLCC68_RESET, LOW);
  delay(10);
  digitalWrite(LLCC68_RESET, HIGH);
  delay(10);
  return LLCC68_HAL_STATUS_OK;
}

llcc68_hal_status_t llcc68_hal_wakeup(const void* context) {
  digitalWrite(LLCC68_NSS, LOW);
  delayMicroseconds(200);
  digitalWrite(LLCC68_NSS, HIGH);
  return LLCC68_HAL_STATUS_OK;
}

// Helper function for initialization error handling
void handleInitError(const char* errorMsg) {
  Serial.print("Init Error: ");
  Serial.println(errorMsg);

  // Optional: Setup error LED blinking
  // pinMode(ERROR_LED_PIN, OUTPUT);
  // while(1) {
  //   digitalWrite(ERROR_LED_PIN, HIGH); delay(200);
  //   digitalWrite(ERROR_LED_PIN, LOW); delay(200);
  // }

  // Enable WDT for auto-reset after ~2s
#ifdef __AVR_ATtiny1606__
  wdt_enable(WDTO_2S);  // ~2s timeout
#else
  _PROTECTED_WRITE(WDT.CTRLA, WDT_PERIOD_2KCLK_gc);  // ~2s timeout at 32kHz
#endif

  while (1) {
    delay(1);
  }
}

// Function to send custom data via LoRa (non-blocking, integrates with state machine)
void send_lora_data(const uint8_t* data, uint8_t length) {
  if (length > 32) {
    Serial.println("Send Error: Data too long (>32 bytes)");
    return;
  }

  // Store custom payload and length
  memset(payload, 0, 32);
  memcpy(payload, data, length);
  // Transition to custom TX setup state
  radio_state = STATE_TX_SETUP;
  state_start_time = millis();
  Serial.println(" TX requested");

}


void llcc68_init() {

  Serial.println("LLCC68 Transceiver Start");

  // Initialize pins
  pinMode(LLCC68_NSS, OUTPUT);
  pinMode(LLCC68_RESET, OUTPUT);
  pinMode(LLCC68_BUSY, INPUT);
  pinMode(LLCC68_DIO1, INPUT);
  digitalWrite(LLCC68_NSS, HIGH);
  digitalWrite(LLCC68_RESET, HIGH);

  // Initialize SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);  // 1.25 MHz SPI clock (20 MHz / 16)

  // Reset radio
  llcc68_hal_status_t hal_status = llcc68_hal_reset(&llcc68_context);
  if (hal_status != LLCC68_HAL_STATUS_OK) {
    handleInitError("Reset Fail");
  }
  delay(10);
  Serial.println("Reset OK");

  // Set to standby
  llcc68_status_t status = llcc68_set_standby(&llcc68_context, LLCC68_STANDBY_CFG_RC);
  if (status != LLCC68_STATUS_OK) {
    handleInitError("Standby Fail");
  }
  Serial.println("Standby OK");

  // Configure LoRa modulation
  status = llcc68_set_pkt_type(&llcc68_context, LLCC68_PKT_TYPE_LORA);
  if (status != LLCC68_STATUS_OK) {
    handleInitError("Pkt Type Fail");
  }
  Serial.println("LoRa OK");

  // Set frequency to 867 MHz
  status = llcc68_set_rf_freq(&llcc68_context, 867000000);
  if (status != LLCC68_STATUS_OK) {
    handleInitError("Freq Fail");
  }
  Serial.println("867 MHz OK");

  // Set modulation parameters: SF9, BW 125 kHz, CR 4/5
  llcc68_mod_params_lora_t mod_params = {
    .sf = LLCC68_LORA_SF9,
    .bw = LLCC68_LORA_BW_125,
    .cr = LLCC68_LORA_CR_4_5
  };
  status = llcc68_set_lora_mod_params(&llcc68_context, &mod_params);
  if (status != LLCC68_STATUS_OK) {
    handleInitError("Mod Params Fail");
  }
  Serial.println("Mod OK");

  // Set packet parameters: 12-symbol preamble, explicit header, 32-byte payload, CRC on
  llcc68_pkt_params_lora_t pkt_params = {
    .preamble_len_in_symb = 12,
    .header_type = LLCC68_LORA_PKT_EXPLICIT,
    .pld_len_in_bytes = 32,
    .crc_is_on = 1,
    .invert_iq_is_on = 0
  };
  status = llcc68_set_lora_pkt_params(&llcc68_context, &pkt_params);
  if (status != LLCC68_STATUS_OK) {
    handleInitError("Pkt Params Fail");
  }
  Serial.println("Pkt OK");

  // Configure high-power PA settings
  llcc68_pa_cfg_params_t pa_params = {
    .pa_duty_cycle = 0x04,
    .hp_max = 0x05,  // +17 dBm
    .device_sel = 0x00,
    .pa_lut = 0x01
  };
  status = llcc68_set_pa_cfg(&llcc68_context, &pa_params);
  if (status != LLCC68_STATUS_OK) {
    handleInitError("PA Fail");
  }
  Serial.println("PA OK");

  // Set TX parameters: 17 dBm
  status = llcc68_set_tx_params(&llcc68_context, 17, LLCC68_RAMP_10_US);
  if (status != LLCC68_STATUS_OK) {
    handleInitError("TX Params Fail");
  }
  Serial.println("TX OK");

  // Set buffer base address
  status = llcc68_set_buffer_base_address(&llcc68_context, 0, 0);
  if (status != LLCC68_STATUS_OK) {
    handleInitError("Buf Fail");
  }
  Serial.println("Buf OK");

  // Configure DIO1 for TX done and RX done interrupts
  llcc68_irq_mask_t irq_mask = LLCC68_IRQ_TX_DONE | LLCC68_IRQ_RX_DONE;
  status = llcc68_set_dio_irq_params(&llcc68_context, irq_mask, irq_mask, 0, 0);
  if (status != LLCC68_STATUS_OK) {
    handleInitError("IRQ Fail");
  }
  Serial.println("IRQ OK");

  // Attach interrupt
  attachInterrupt(digitalPinToInterrupt(LLCC68_DIO1), dio1_isr, RISING);
  Serial.println("Int OK");

  // Initialization complete
  Serial.println("LLCC68 Init Complete!");
  radio_state = STATE_IDLE;
  state_start_time = millis();
  stop_transmission = false;  // Ensure flag is reset
}

void llcc68_func() {
  llcc68_status_t status;

  switch (radio_state) {

    case STATE_IDLE:
      if (millis() - state_start_time >= IDLE_DURATION_MS) {
        radio_state = STATE_RX_SETUP;
        state_start_time = millis();
      }
      break;

    case STATE_TX_SETUP:
      // Prepare and write default payload
      memcpy(buffer, payload, 32);
      status = llcc68_write_buffer(&llcc68_context, 0, buffer, 32);
      if (status != LLCC68_STATUS_OK) {
        Serial.println("Buf Wr Fail");
        radio_state = STATE_IDLE;
        state_start_time = millis();
        break;
      }
      Serial.println("Buf Wr OK");
      Serial.println((char*)buffer);
      // Start transmission
      dio1_triggered = false;
      status = llcc68_set_tx(&llcc68_context, 0);
      if (status != LLCC68_STATUS_OK) {
        Serial.println("TX Fail");
        radio_state = STATE_IDLE;
        state_start_time = millis();
        break;
      }
      Serial.println("TX Start");
      radio_state = STATE_TX_WAIT;
      state_start_time = millis();
      break;

    case STATE_TX_WAIT:
      if (dio1_triggered) {
        Serial.println("TX Done");
        status = llcc68_clear_irq_status(&llcc68_context, LLCC68_IRQ_TX_DONE);
        if (status != LLCC68_STATUS_OK) {
          Serial.println("TX IRQ Fail");
        } else {
          Serial.println("TX IRQ OK");
        }
        radio_state = STATE_RX_SETUP;
        state_start_time = millis();
      } else if (millis() - state_start_time >= TX_TIMEOUT_MS) {
        Serial.println("TX Timeout");
        llcc68_clear_irq_status(&llcc68_context, LLCC68_IRQ_ALL);
        radio_state = STATE_IDLE;
        state_start_time = millis();
      }
      break;

    case STATE_RX_SETUP:
      dio1_triggered = false;
      status = llcc68_set_rx(&llcc68_context, 5000);
      if (status != LLCC68_STATUS_OK) {
        Serial.println("RX Fail");
        radio_state = STATE_IDLE;
        state_start_time = millis();
        break;
      }
      Serial.println("RX Start");
      radio_state = STATE_RX_WAIT;
      state_start_time = millis();
      break;

    case STATE_RX_WAIT:
      if (dio1_triggered) {
        llcc68_irq_mask_t irq_status;
        status = llcc68_get_irq_status(&llcc68_context, &irq_status);
        if (status != LLCC68_STATUS_OK) {
          Serial.println("IRQ Stat Fail");
          radio_state = STATE_IDLE;
          state_start_time = millis();
          break;
        }
        if (irq_status & LLCC68_IRQ_RX_DONE) {
          Serial.println("RX Done");
          uint8_t rx_buffer[32];
          uint8_t rx_length = 0;
          llcc68_rx_buffer_status_t rx_status;
          status = llcc68_get_rx_buffer_status(&llcc68_context, &rx_status);
          if (status == LLCC68_STATUS_OK) {
            rx_length = rx_status.pld_len_in_bytes;
            status = llcc68_read_buffer(&llcc68_context, rx_status.buffer_start_pointer, rx_buffer, rx_length);
            if (status == LLCC68_STATUS_OK) {
              Serial.print("RX: ");
              for (uint8_t i = 0; i < rx_length; i++) {
                Serial.print((char)rx_buffer[i]);
              }
              Serial.println();

              int16_t rssi;
              status = llcc68_get_rssi_inst(&llcc68_context, &rssi);
              if (status == LLCC68_STATUS_OK) {
                Serial.print("RSSI: ");
                Serial.print(rssi);
                Serial.println(" dBm");
              } else {
                Serial.println("RSSI Fail");
              }
            } else {
              Serial.println("Buf Rd Fail");
            }
          } else {
            Serial.println("Buf Stat Fail");
          }

          status = llcc68_clear_irq_status(&llcc68_context, LLCC68_IRQ_RX_DONE);
          if (status != LLCC68_STATUS_OK) {
            Serial.println("RX IRQ Fail");
          }
        } else {
          Serial.println("No RX IRQ");
        }
        radio_state = STATE_IDLE;
        state_start_time = millis();
      } else if (millis() - state_start_time >= RX_TIMEOUT_MS) {
        Serial.println("RX Timeout");
        radio_state = STATE_IDLE;
        state_start_time = millis();
      }
      break;
  }
}