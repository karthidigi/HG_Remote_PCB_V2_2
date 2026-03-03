// =============================================
// sx1268Main.h – SX1262/SX1268 LoRa Radio Driver (Remote ATtiny1606)
// Replaces: llcc68Main.h
// =============================================
// Configuration: SF12, BW250, CR4/8, Preamble=16, Sync=0x1424 (private)
// Frequency: 867 MHz,  TX Power: 22 dBm
// Expected Range: ~8–12 km (line of sight, optimal conditions)
// Time-on-Air: ~1380 ms per 32-byte packet   (SF10: ~345 ms)
// NOTE: MUST MATCH STARTER SETTINGS!
//
// SX126x family (SX1262/SX1268) supports all SF/BW combinations.
// SF12: symbol time 16.4 ms → LDRO must be ON.

#ifndef SX1268_MAIN_H
#define SX1268_MAIN_H

#include "zSettings.h"          // PAIR_*/OPER_*/PKT_* constants
#include "hwPins.h"              // SX1268_NSS, SX1268_BUSY, SX1268_RESET_PIN, SX1268_DIO1
#include <SPI.h>
#include "src/SX1268_driver.h"

volatile bool dio1_triggered = false;

// ────────────────────────────────────────────────
// Forward declarations
// ────────────────────────────────────────────────
void send_lora_data(const uint8_t* data, uint8_t length);
void sx1268Init();
void sx1268Func();
void switchToPairChannel();
void switchToOperationalChannel();
// dispatchPairPkt is defined in pairRemoteNode.h (included after sx1268Main.h)
void dispatchPairPkt(const uint8_t* buf, uint8_t len);

// ────────────────────────────────────────────────
// Radio state machine
// ────────────────────────────────────────────────
enum RadioState {
    STATE_IDLE,
    STATE_TX_SETUP,
    STATE_TX_WAIT,
    STATE_RX_SETUP,
    STATE_RX_WAIT
};

// ────────────────────────────────────────────────
// Globals
// ────────────────────────────────────────────────
static uint8_t radioBuf[32];        // shared TX/RX buffer
#define payload     radioBuf        // alias for TX
#define bufferRadio radioBuf        // alias for RX

static unsigned long state_start_time = 0;
static const unsigned long TX_TIMEOUT_MS    = 5000UL;   // SF11/BW125 ToA ~2.5s + margin
static const unsigned long RX_TIMEOUT_MS    = 8000UL;   // 8 sec RX window for reliable ACK
static const unsigned long IDLE_DURATION_MS = 100UL;    // Fast return to RX mode

static RadioState radio_state = STATE_IDLE;

// ────────────────────────────────────────────────
// ISR – Arduino attachInterrupt (ATtiny1606 PA7)
// ────────────────────────────────────────────────
static void dio1_isr() {
    dio1_triggered = true;
}

// ────────────────────────────────────────────────
// Non-blocking send (same interface as before)
// ────────────────────────────────────────────────
void send_lora_data(const uint8_t* data, uint8_t length) {
    if (length > 32) return;
    if (!bit_is_set(SREG, 7)) sei();
    memset(payload, 0, sizeof(payload));
    memcpy(payload, data, length);
    radio_state      = STATE_TX_SETUP;
    state_start_time = millis();
}

// ────────────────────────────────────────────────
// Channel-switch helpers
// ────────────────────────────────────────────────

// Switch to SF7/BW125/CR4-5/pre=8/sync=0x1234/14dBm  (LDRO=0)
void switchToPairChannel() {
    SX1268_setStandby(SX1268_STANDBY_RC);
    SX1268_setModulationParamsLoRa(PAIR_SF, PAIR_BW, PAIR_CR, 0); // LDRO=0
    SX1268_setPacketParamsLoRa(PAIR_PREAMBLE, SX1268_HEADER_EXPLICIT, 32,
                               SX1268_CRC_ON, SX1268_IQ_STANDARD);
    // SX126x errata: re-apply IQ register after every setPacketParams call
    SX1268_fixInvertedIq(SX1268_IQ_STANDARD);
    uint8_t sw[2] = { PAIR_SYNC_MSB, PAIR_SYNC_LSB };
    SX1268_writeRegister(SX1268_REG_LORA_SYNC_WORD_MSB, sw, 2);
    SX1268_setTxParams(PAIR_TX_POWER, SX1268_PA_RAMP_200U);
    SX1268_clearIrqStatus(SX1268_IRQ_ALL);
    dio1_triggered   = false;
    radio_state      = STATE_IDLE;
    state_start_time = millis();
    DEBUG_PRINTN("Radio: switched to PAIR channel (SF7/BW125/0x1234)");
}

// Switch to SF11/BW125/CR4-5/pre=12/sync=0x3444/22dBm  (LDRO=1)
void switchToOperationalChannel() {
    SX1268_setStandby(SX1268_STANDBY_RC);
    SX1268_setModulationParamsLoRa(OPER_SF, OPER_BW, OPER_CR, 1); // LDRO=1
    SX1268_setPacketParamsLoRa(OPER_PREAMBLE, SX1268_HEADER_EXPLICIT, 32,
                               SX1268_CRC_ON, SX1268_IQ_STANDARD);
    // SX126x errata: re-apply IQ register after every setPacketParams call
    SX1268_fixInvertedIq(SX1268_IQ_STANDARD);
    uint8_t sw[2] = { OPER_SYNC_MSB, OPER_SYNC_LSB };
    SX1268_writeRegister(SX1268_REG_LORA_SYNC_WORD_MSB, sw, 2);
    SX1268_setTxParams(OPER_TX_POWER, SX1268_PA_RAMP_200U);
    SX1268_clearIrqStatus(SX1268_IRQ_ALL);
    dio1_triggered   = false;
    radio_state      = STATE_IDLE;
    state_start_time = millis();
    DEBUG_PRINTN("Radio: switched to OPER channel (SF11/BW125/0x3444)");
}

// ────────────────────────────────────────────────
// Radio init
// ────────────────────────────────────────────────
void sx1268Init() {
    // Register SPI bus and pins with driver
    SX1268_setSPI(SPI, 0);
    SX1268_setPins(SX1268_NSS, SX1268_BUSY);

    SX1268_begin();

    // Hardware reset
    SX1268_reset(SX1268_RESET_PIN);
    delay(10);

    SX1268_setStandby(SX1268_STANDBY_RC);

    // TCXO: configure before calibration
    SX1268_setDio3AsTcxoCtrl(SX1268_DIO3_OUTPUT_1_8, SX1268_TCXO_DELAY_10);
    SX1268_calibrate(0xFF);
    SX1268_setStandby(SX1268_STANDBY_RC);

    SX1268_setRegulatorMode(SX1268_REGULATOR_DC_DC);
    SX1268_calibrateImage(SX1268_CAL_IMG_863, SX1268_CAL_IMG_870);
    SX1268_setDio2AsRfSwitchCtrl(SX1268_DIO2_AS_RF_SWITCH);
    SX1268_setPacketType(SX1268_LORA_MODEM);

    // Frequency: 867 MHz
    uint32_t rfFreq = ((uint64_t)867000000UL << SX1268_RF_FREQUENCY_SHIFT)
                      / SX1268_RF_FREQUENCY_XTAL;
    SX1268_setRfFrequency(rfFreq);

    SX1268_setPaConfig(0x04, 0x07, 0x00, 0x01);
    SX1268_setTxParams(22, SX1268_PA_RAMP_200U);

    // Init channel: SF12/BW250/CR4-8/LDRO=1
    SX1268_setModulationParamsLoRa(12, SX1268_BW_250000, SX1268_CR_4_8, 1);

    SX1268_setPacketParamsLoRa(16, SX1268_HEADER_EXPLICIT, 32,
                               SX1268_CRC_ON, SX1268_IQ_STANDARD);
    SX1268_fixInvertedIq(SX1268_IQ_STANDARD);

    // Sync word: private LoRa (0x12) → {0x14, 0x24}
    uint8_t sw[2] = {0x14, 0x24};
    SX1268_writeRegister(SX1268_REG_LORA_SYNC_WORD_MSB, sw, 2);

    SX1268_setBufferBaseAddress(0x00, 0x80);
    SX1268_fixResistanceAntenna();

    uint16_t irq_mask = SX1268_IRQ_TX_DONE  | SX1268_IRQ_RX_DONE   |
                        SX1268_IRQ_TIMEOUT   | SX1268_IRQ_HEADER_ERR |
                        SX1268_IRQ_CRC_ERR;
    SX1268_setDioIrqParams(irq_mask, irq_mask,
                           SX1268_IRQ_NONE,  SX1268_IRQ_NONE);

    // DIO1 rising-edge interrupt (ATtiny1606)
    pinMode(SX1268_DIO1, INPUT);
    attachInterrupt(digitalPinToInterrupt(SX1268_DIO1), dio1_isr, RISING);

    radio_state      = STATE_IDLE;
    state_start_time = millis();
    dio1_triggered   = false;
}

// ────────────────────────────────────────────────
// State machine (called every loop iteration)
// ────────────────────────────────────────────────
void sx1268Func() {
    switch (radio_state) {

        case STATE_IDLE:
            if (millis() - state_start_time >= IDLE_DURATION_MS) {
                radio_state      = STATE_RX_SETUP;
                state_start_time = millis();
            }
            break;

        case STATE_TX_SETUP: {
            // FIX: Always go through standby before TX.
            // If radio is in timed RX, issuing SetTx directly can silently drop
            // the opcode (BUSY briefly HIGH during RX→TX transition).
            SX1268_setStandby(SX1268_STANDBY_RC);
            SX1268_setBufferBaseAddress(0x00, 0x80);
            SX1268_writeBuffer(0x00, bufferRadio, 32);
            dio1_triggered = false;
            SX1268_clearIrqStatus(SX1268_IRQ_ALL);
            SX1268_setTx(SX1268_TX_SINGLE);
            radio_state      = STATE_TX_WAIT;
            state_start_time = millis();
            break;
        }

        case STATE_TX_WAIT: {
            if (dio1_triggered) {
                SX1268_clearIrqStatus(SX1268_IRQ_ALL);
                dio1_triggered   = false;
                radio_state      = STATE_RX_SETUP;
                state_start_time = millis();
            } else if (millis() - state_start_time >= TX_TIMEOUT_MS) {
                SX1268_clearIrqStatus(SX1268_IRQ_ALL);
                radio_state      = STATE_IDLE;
                state_start_time = millis();
            }
            break;
        }

        case STATE_RX_SETUP: {
            dio1_triggered = false;
            SX1268_setBufferBaseAddress(0x00, 0x80);
            SX1268_clearIrqStatus(SX1268_IRQ_ALL);
            // 8-second RX window
            SX1268_setRx((uint32_t)(RX_TIMEOUT_MS * 64UL));
            radio_state      = STATE_RX_WAIT;
            state_start_time = millis();
            break;
        }

        case STATE_RX_WAIT: {
            if (dio1_triggered) {
                uint16_t irq_status;
                SX1268_getIrqStatus(&irq_status);

                // FIX: Only process packet when RX_DONE AND no CRC error.
                // SX1268 sets both RX_DONE and CRC_ERR together on bad packets.
                if ((irq_status & SX1268_IRQ_RX_DONE) &&
                    !(irq_status & SX1268_IRQ_CRC_ERR)) {
                    uint8_t rx_length, rx_start;
                    SX1268_getRxBufferStatus(&rx_length, &rx_start);
                    SX1268_fixRxTimeout();

                    if (rx_length > 0 && rx_length <= 32) {
                        uint8_t rx_buffer[32];
                        SX1268_readBuffer(rx_start, rx_buffer, rx_length);

                        if (rx_buffer[0] >= PKT_PAIR_REQ &&
                            rx_buffer[0] <= PKT_REM_PAIR_DONE) {
                            dispatchPairPkt(rx_buffer, rx_length);
                        } else {
                            decryptNFunc(rx_buffer, rx_length);
                        }
                    }
                }

                SX1268_clearIrqStatus(SX1268_IRQ_ALL);
                dio1_triggered   = false;
                radio_state      = STATE_IDLE;
                state_start_time = millis();
            } else if (millis() - state_start_time >= RX_TIMEOUT_MS) {
                SX1268_clearIrqStatus(SX1268_IRQ_ALL);
                radio_state      = STATE_IDLE;
                state_start_time = millis();
            }
            break;
        }
    }
}

#endif // SX1268_MAIN_H
