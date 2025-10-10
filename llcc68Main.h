// ---------- FILE: llcc68Main.h ----------
#include <SPI.h>
#include "src/llcc68.h"
#include "src/llcc68_hal.h"


// static uint8_t payload[32];
// static uint8_t bufferRadio[32];

static uint8_t radioBuf[32];   // Shared TX/RX buffer
#define payload   radioBuf     // alias for TX
#define bufferRadio radioBuf   // alias for RX

static unsigned long state_start_time = 0;
static const unsigned long TX_TIMEOUT_MS = 5000UL;
static const unsigned long RX_TIMEOUT_MS = 5000UL;
static const unsigned long IDLE_DURATION_MS = 1000UL;
volatile bool dio1_triggered = false;

// Context placeholder
typedef struct {
  uint8_t dummy;
} llcc68_context_t;

static llcc68_context_t llcc68_context;

static inline void dio1_isr() {
  dio1_triggered = true;
  //DEBUG_PRINTN("DIO1 Triggered");
}

enum RadioState { STATE_IDLE,
                  STATE_TX_SETUP,
                  STATE_TX_WAIT,
                  STATE_RX_SETUP,
                  STATE_RX_WAIT };

static RadioState radio_state = STATE_IDLE;


// HAL write/read/reset/wakeup - minimal
llcc68_hal_status_t llcc68_hal_write(const void* context, const uint8_t* command, const uint16_t command_length,
                                     const uint8_t* data, const uint16_t data_length) {
  digitalWrite(LLCC68_NSS, LOW);
  for (uint16_t i = 0; i < command_length; i++) SPI.transfer(command[i]);
  for (uint16_t i = 0; i < data_length; i++) SPI.transfer(data[i]);
  digitalWrite(LLCC68_NSS, HIGH);
  return LLCC68_HAL_STATUS_OK;
}

llcc68_hal_status_t llcc68_hal_read(const void* context, const uint8_t* command, const uint16_t command_length,
                                    uint8_t* data, const uint16_t data_length) {
  digitalWrite(LLCC68_NSS, LOW);
  for (uint16_t i = 0; i < command_length; i++) SPI.transfer(command[i]);
  for (uint16_t i = 0; i < data_length; i++) data[i] = SPI.transfer(0x00);
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

// Non-blocking send
static inline void send_lora_data(const uint8_t* data, uint8_t length) {
  if (length > 32) {
    //DEBUG_PRINTN("Send Error: Data too long (>32 bytes)");
    return;
  }
  if (!bit_is_set(SREG, 7)) {
    //DEBUG_PRINTN(F("Interrupts are OFF, enabling now"));
    sei();
  }
  memset(payload, 0, sizeof(payload));
  memcpy(payload, data, length);
  radio_state = STATE_TX_SETUP;
  state_start_time = millis();
  //DEBUG_PRINTN(" TX requested");
}



static inline void llcc68Init() {
  pinMode(LLCC68_NSS, OUTPUT);
  pinMode(LLCC68_RESET, OUTPUT);
  pinMode(LLCC68_BUSY, INPUT);
  pinMode(LLCC68_DIO1, INPUT);
  digitalWrite(LLCC68_NSS, HIGH);
  digitalWrite(LLCC68_RESET, HIGH);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  if (llcc68_hal_reset(&llcc68_context) != LLCC68_HAL_STATUS_OK) {
    //DEBUG_PRINTN("Reset Fail");
   delay(10);
  }

  // Minimal configuration - keep same as before to maintain behavior
  if (llcc68_set_standby(&llcc68_context, LLCC68_STANDBY_CFG_RC) != LLCC68_STATUS_OK) {
   delay(10);
    //DEBUG_PRINTN("STAND BY Fail");
  }
  if (llcc68_set_pkt_type(&llcc68_context, LLCC68_PKT_TYPE_LORA) != LLCC68_STATUS_OK) {
   delay(10);
    //DEBUG_PRINTN("PKT TYPE Fail");
  }
  if (llcc68_set_rf_freq(&llcc68_context, 867000000) != LLCC68_STATUS_OK) {
   delay(10);
    //DEBUG_PRINTN("FREQ Fail");
  }

  // Mod params
  llcc68_mod_params_lora_t mod_params = {
    .sf = LLCC68_LORA_SF9,
    .bw = LLCC68_LORA_BW_125,
    .cr = LLCC68_LORA_CR_4_5,
     .ldro = 1       
  };
  
  llcc68_set_lora_mod_params(&llcc68_context, &mod_params);

  // Packet params
  llcc68_pkt_params_lora_t pkt_params = { 
    .preamble_len_in_symb = 12, 
    .header_type = LLCC68_LORA_PKT_EXPLICIT, 
    .pld_len_in_bytes = 32, 
    .crc_is_on = 1, 
    .invert_iq_is_on = 0 
    };

  llcc68_set_lora_pkt_params(&llcc68_context, &pkt_params);

  // PA
  llcc68_pa_cfg_params_t pa_params = { 
    .pa_duty_cycle = 0x04, 
    .hp_max = 0x07,  //0x05 old 
    .device_sel = 0x00, 
    .pa_lut = 0x01 };

  llcc68_set_pa_cfg(&llcc68_context, &pa_params);
  llcc68_set_tx_params(&llcc68_context, 22, LLCC68_RAMP_200_US);
  llcc68_set_buffer_base_address(&llcc68_context, 0, 0);

  llcc68_irq_mask_t irq_mask = LLCC68_IRQ_TX_DONE | LLCC68_IRQ_RX_DONE;
  llcc68_set_dio_irq_params(&llcc68_context, irq_mask, irq_mask, 0, 0);

  attachInterrupt(digitalPinToInterrupt(LLCC68_DIO1), dio1_isr, RISING);

  radio_state = STATE_IDLE;
  state_start_time = millis();
}



// Radio state machine - call from loop()
static inline void llcc68Func() {
  llcc68_status_t status;
  switch (radio_state) {
    case STATE_IDLE:
      if (millis() - state_start_time >= IDLE_DURATION_MS) {
        radio_state = STATE_RX_SETUP;
        state_start_time = millis();
      }
      break;

    case STATE_TX_SETUP:
      memcpy(bufferRadio, payload, 32);
      status = llcc68_write_buffer(&llcc68_context, 0, bufferRadio, 32);
      if (status != LLCC68_STATUS_OK) {
        //DEBUG_PRINTN("Buf Wr Fail");
        radio_state = STATE_IDLE;
        state_start_time = millis();
        break;
      }
      dio1_triggered = false;
      status = llcc68_set_tx(&llcc68_context, 0);
      if (status != LLCC68_STATUS_OK) {
        //DEBUG_PRINTN("TX Fail");
        radio_state = STATE_IDLE;
        state_start_time = millis();
        break;
      }
      radio_state = STATE_TX_WAIT;
      state_start_time = millis();
      break;

    case STATE_TX_WAIT:
      if (dio1_triggered) {
        //DEBUG_PRINTN("TX Done");
        llcc68_clear_irq_status(&llcc68_context, LLCC68_IRQ_TX_DONE);
        radio_state = STATE_RX_SETUP;
        state_start_time = millis();
      } else if (millis() - state_start_time >= TX_TIMEOUT_MS) {
        //DEBUG_PRINTN("TX Timeout");
        llcc68_clear_irq_status(&llcc68_context, LLCC68_IRQ_ALL);
        radio_state = STATE_IDLE;
        state_start_time = millis();
      }
      break;

    case STATE_RX_SETUP:
      dio1_triggered = false;
      status = llcc68_set_rx(&llcc68_context, 5000);
      if (status != LLCC68_STATUS_OK) {
        //DEBUG_PRINTN("RX Fail");
        radio_state = STATE_IDLE;
        state_start_time = millis();
        break;
      }
      radio_state = STATE_RX_WAIT;
      state_start_time = millis();
      break;

    case STATE_RX_WAIT:
      if (dio1_triggered) {
        llcc68_irq_mask_t irq_status;
        status = llcc68_get_irq_status(&llcc68_context, &irq_status);
        if (status == LLCC68_STATUS_OK && (irq_status & LLCC68_IRQ_RX_DONE)) {
          llcc68_rx_buffer_status_t rx_status;
          status = llcc68_get_rx_buffer_status(&llcc68_context, &rx_status);
          if (status == LLCC68_STATUS_OK) {
            uint8_t rx_length = rx_status.pld_len_in_bytes;
            uint8_t rx_buffer[32];
            status = llcc68_read_buffer(&llcc68_context, rx_status.buffer_start_pointer, rx_buffer, rx_length);
            if (status == LLCC68_STATUS_OK) {
              // DEBUG_PRINT(F("RX raw: ["));
              // for (uint8_t i = 0; i < rx_length; ++i) {
              // DEBUG_PRINT((char)rx_buffer[i]);
              // }
              // DEBUG_PRINTN("]");
              decryptNFunc(rx_buffer, rx_length);
            }
          }
          llcc68_clear_irq_status(&llcc68_context, LLCC68_IRQ_RX_DONE);
        } else {
         delay(10);
          //DEBUG_PRINTN("No RX IRQ or IRQ read fail");
        }
        radio_state = STATE_IDLE;
        state_start_time = millis();
      } else if (millis() - state_start_time >= RX_TIMEOUT_MS) {
        //DEBUG_PRINTN("RX Timeout");
        radio_state = STATE_IDLE;
        state_start_time = millis();
      }
      break;
  }
}



