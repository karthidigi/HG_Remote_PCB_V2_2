#pragma once
/////////////////////////////////////////////////////
#define FIRMWARE_VERSION        "ATtiny1606"
#define HARDWARE_VERSION        "1.1.1"

/////////////////////////////////////////////////////
#define SERIAL_DEBUG
#define SERIAL_BAUD     115200
#define SERIAL_TIMEOUT  100


/////////////////////////////////////////////////////
#define WATCHDOG 1

/////////////////////////////////////////////////////
#define LP_TIMEOUT_MS (30UL * 1000UL) // 1 minutes

/////////////////////////////////////////////////////
#define BUZZ_NOR 120
#define BUZZ_OFF 600


//////////////////////////////////////////////////////
#define BAT_VOL_MIN 2200


//////////////////////////////////////////////////////
#define MOT_CONT_ID "42407197000090220136"

// ── LoRa Pairing channel (SF7, distinct sync word) ──
// MUST match Starter (AVRDB_LLCC_V2_2_GSM_But_LCD/zSettings.h)
#define PAIR_SF           7
#define PAIR_BW           SX1268_BW_125000
#define PAIR_CR           SX1268_CR_4_5
#define PAIR_PREAMBLE     8
#define PAIR_SYNC_MSB     0x12
#define PAIR_SYNC_LSB     0x34
#define PAIR_TX_POWER     14
// LDRO: SF7+BW125 → symbol time 1.0 ms < 16 ms → OFF (literal 0)

// ── Operational channel (MUST match Starter) ──
#define OPER_SF           11
#define OPER_BW           SX1268_BW_125000
#define OPER_CR           SX1268_CR_4_5
#define OPER_PREAMBLE     12
#define OPER_SYNC_MSB     0x34
#define OPER_SYNC_LSB     0x44
#define OPER_TX_POWER     22
// LDRO: SF11+BW125 → symbol time 16.4 ms >= 16 ms → ON (literal 1)

// ── Pairing packet types ──
#define PKT_PAIR_REQ      0x0A   // Starter → Gateway
#define PKT_PAIR_ACK      0x0B   // Gateway → Starter
#define PKT_PAIR_DONE     0x0C   // Starter → Gateway
#define PKT_REM_PAIR_REQ  0x0D   // Remote  → Starter  [type][rem_serial:20]
#define PKT_REM_PAIR_ACK  0x0E   // Starter → Remote   [type][starter_id:20][sf][bwCode][cr][pre][pwr]
#define PKT_REM_PAIR_DONE 0x0F   // Remote  → Starter  [type][serial_echo:4]

// ── Pairing timing ──
#define PAIR_BEACON_INTERVAL_MS  2000UL
#define PAIR_SCAN_WINDOW_MS     60000UL
#define PAIR_ACK_TIMEOUT_MS      5000UL
#define PAIR_ACK_MAX_RETRIES         3
                     