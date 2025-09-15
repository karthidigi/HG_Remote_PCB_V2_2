// ---------- FILE: llcc68Main.h ----------
#pragma once
#include <SPI.h>
#include "src/llcc68.h"
#include "src/llcc68_hal.h"

// Context placeholder
typedef struct {
  uint8_t dummy;
} llcc68_context_t;
static llcc68_context_t llcc68_context;

volatile bool dio1_triggered = false;
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
static unsigned long state_start_time = 0;
static const unsigned long TX_TIMEOUT_MS = 5000UL;
static const unsigned long RX_TIMEOUT_MS = 5000UL;
static const unsigned long IDLE_DURATION_MS = 1000UL;

static uint8_t payload[32];
static uint8_t bufferRadio[32];

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
  memset(payload, 0, sizeof(payload));
  memcpy(payload, data, length);
  radio_state = STATE_TX_SETUP;
  state_start_time = millis();
   //DEBUG_PRINTN(" TX requested");
}

static inline void llcc68_init() {
  pinMode(LLCC68_NSS, OUTPUT);
  pinMode(LLCC68_RESET, OUTPUT);
  pinMode(LLCC68_BUSY, INPUT);
  pinMode(LLCC68_DIO1, INPUT);
  digitalWrite(LLCC68_NSS, HIGH);
  digitalWrite(LLCC68_RESET, HIGH);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);

  if (llcc68_hal_reset(&llcc68_context) != LLCC68_HAL_STATUS_OK)  ;//DEBUG_PRINTN("Reset Fail");
  delay(10);

  // Minimal configuration - keep same as before to maintain behavior
  if (llcc68_set_standby(&llcc68_context, LLCC68_STANDBY_CFG_RC) != LLCC68_STATUS_OK)  ;//DEBUG_PRINTN("STAND BY Fail");
  if (llcc68_set_pkt_type(&llcc68_context, LLCC68_PKT_TYPE_LORA) != LLCC68_STATUS_OK)  ;//DEBUG_PRINTN("PKT TYPE Fail");
  if (llcc68_set_rf_freq(&llcc68_context, 867000000) != LLCC68_STATUS_OK)  ;//DEBUG_PRINTN("FREQ Fail");

  // Mod params
  llcc68_mod_params_lora_t mod_params = { .sf = LLCC68_LORA_SF9, .bw = LLCC68_LORA_BW_125, .cr = LLCC68_LORA_CR_4_5 };
  llcc68_set_lora_mod_params(&llcc68_context, &mod_params);

  // Packet params
  llcc68_pkt_params_lora_t pkt_params = { .preamble_len_in_symb = 12, .header_type = LLCC68_LORA_PKT_EXPLICIT, .pld_len_in_bytes = 32, .crc_is_on = 1, .invert_iq_is_on = 0 };
  llcc68_set_lora_pkt_params(&llcc68_context, &pkt_params);

  // PA
  llcc68_pa_cfg_params_t pa_params = { .pa_duty_cycle = 0x04, .hp_max = 0x05, .device_sel = 0x00, .pa_lut = 0x01 };
  llcc68_set_pa_cfg(&llcc68_context, &pa_params);
  llcc68_set_tx_params(&llcc68_context, 17, LLCC68_RAMP_10_US);
  llcc68_set_buffer_base_address(&llcc68_context, 0, 0);

  llcc68_irq_mask_t irq_mask = LLCC68_IRQ_TX_DONE | LLCC68_IRQ_RX_DONE;
  llcc68_set_dio_irq_params(&llcc68_context, irq_mask, irq_mask, 0, 0);

  attachInterrupt(digitalPinToInterrupt(LLCC68_DIO1), dio1_isr, RISING);

  radio_state = STATE_IDLE;
  state_start_time = millis();
}



// Receiver-side: decrypt incoming LoRa buffer
static inline void RxLoraData(const uint8_t* rx_buf, uint8_t rx_len) {

  if (rx_len == 0 || rx_len > 64) return;

  char hexBuf[65];
  uint8_t copyLen = (rx_len < sizeof(hexBuf) - 1) ? rx_len : sizeof(hexBuf) - 1;
  memcpy(hexBuf, rx_buf, copyLen);
  hexBuf[copyLen] = '\0';

  // ---- Parse idx + rnd ----
  if (strlen(hexBuf) < 6) {
     //DEBUG_PRINTLN(F("RX too short for idx+rnd"));
    return;
  }

  // Extract idx (2 hex chars → byte)
  // uint8_t idx = (fromHexChar(hexBuf[0]) << 4) | fromHexChar(hexBuf[1]);

  // Extract rnd (next 4 hex chars → uint16_t)
  uint16_t rnd = (fromHexChar(hexBuf[2]) << 12) | (fromHexChar(hexBuf[3]) << 8) | (fromHexChar(hexBuf[4]) << 4) | (fromHexChar(hexBuf[5]));

  // Remaining ciphertext (ASCII hex)
  const char* cipherHex = hexBuf + 6;

  // ---- Build AES key from *this receiver’s* chip serial ----
  char selfSerial[21];
  getChipSerial(selfSerial, sizeof(selfSerial));
   //DEBUG_PRINTLN(selfSerial);

  uint8_t key[KEY_LEN] = { 0 };
  const char* last12 = selfSerial + 8;  // last 12 chars = 6 bytes
   //DEBUG_PRINTLN(last12);
  for (uint8_t i = 0; i < 6; i++) {
    char hexPair[3] = { last12[i * 2], last12[i * 2 + 1], 0 };
    key[(KEY_LEN - 8) + i] = (uint8_t)strtoul(hexPair, NULL, 16);
  }


  // Append rnd into last 2 bytes
  key[KEY_LEN - 2] = rnd >> 8;
  key[KEY_LEN - 1] = rnd & 0xFF;

//    //DEBUG_PRINT(F("Key: ["));
//   for (uint8_t i = 0; i < KEY_LEN; i++) {
// #ifdef SERIAL_DEBUG
//     if (key[i] < 0x10) Serial.print('0');
//     Serial.print(key[i], HEX);
//     Serial.print(' ');
// #else
//      //DEBUG_PRINT(key[i]);  // fallback, but will look weird
// #endif
//   }
//    //DEBUG_PRINTLN("]");

  // ---- Decrypt ----
  // char decBuf[MAX_MESSAGE_LEN + 1];
  // if (decryptWithIdx(cipherHex, key, selfSerial, decBuf, sizeof(decBuf))) {
  //    //DEBUG_PRINT("Decrypted: ");
  //    //DEBUG_PRINTLN(decBuf);
  // } else {
  //    //DEBUG_PRINTLN("Decrypt failed");
  // }


  char decBuf[MAX_MESSAGE_LEN + 1];

  if (decryptWithIdx(cipherHex, key, selfSerial, decBuf, sizeof(decBuf))) {
    char* start = strchr(decBuf, '[');
    char* end = strchr(decBuf, ']');

    if (start && end && end > start) {
      // Extract text between [ and ]
      size_t len = end - start - 1;  // content length
      char inner[32];                // adjust size as needed
      if (len < sizeof(inner)) {
        strncpy(inner, start + 1, len);
        inner[len] = '\0';  // null terminate

         //DEBUG_PRINT(F("Extracted: "));
         //DEBUG_PRINTLN(inner);
        // Example: check for "m11"
        if (strcmp(inner, "M11") == 0) {
           //DEBUG_PRINTLN(F("M11 received"));
          funcM1LGreen();
          buzBeepNonBlocking(150);
          delay(200);
          buzBeepNonBlocking(150);

        } else if (strcmp(inner, "M21") == 0) {
           //DEBUG_PRINTLN(F("M21 received"));
          funcM2LGreen();
          buzBeepNonBlocking(150);
          delay(200);
          buzBeepNonBlocking(150);
        } else if (strcmp(inner, "M10") == 0) {
           //DEBUG_PRINTLN(F("M10 received"));
          funcM1LRed();
          buzBeepNonBlocking(500);
        } else if (strcmp(inner, "M20") == 0) {
           //DEBUG_PRINTLN(F("M20 received"));
          funcM2LRed();
          buzBeepNonBlocking(500);
        } else if (strcmp(inner, "M00") == 0) {
           //DEBUG_PRINTLN(F("M00 received"));
          funcMLRed();
          buzBeepNonBlocking(500);
        } else {
          funcM1Yellow();
          delay(200);
        }
      } else {
         //DEBUG_PRINTLN(F("Content too long"));
      }
    } else {
       //DEBUG_PRINTLN(F("Invalid message (no brackets)"));
    }
  } else {
     //DEBUG_PRINTLN(F("Decrypt failed"));
    funcStaWhite();
    delay(200);
  }
  funcLedReset();
}



// Radio state machine - call from loop()
static inline void llcc68_func() {
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
               //DEBUG_PRINT(F("RX raw: ["));
              for (uint8_t i = 0; i < rx_length; ++i) {
                 //DEBUG_PRINT((char)rx_buffer[i]);
              }
               //DEBUG_PRINTLN("]");
              RxLoraData(rx_buffer, rx_length);
              // Optionally echo or handle rx_buffer
            }
          }
          llcc68_clear_irq_status(&llcc68_context, LLCC68_IRQ_RX_DONE);
        } else {
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
