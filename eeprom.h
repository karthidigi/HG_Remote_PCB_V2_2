#include <EEPROM.h>

#define EEPROM_PEER_SERIAL_ADDR  0x00   // starting location in EEPROM
#define PEER_SERIAL_LEN          21     // 20 chars + '\0'

// Save peer serial (20-char hex string + null)
static inline void savePeerSerial(const char *buffer) {
  for (uint8_t i = 0; i < PEER_SERIAL_LEN; i++) {
    char c = buffer[i];
    EEPROM.update(EEPROM_PEER_SERIAL_ADDR + i, c);
    if (c == '\0') break;  // stop early if null terminator found
  }
}

// Read peer serial back into buffer
static inline void readPeerSerial(char *buffer, size_t bufferSize) {
  if (bufferSize < PEER_SERIAL_LEN) {
    buffer[0] = '\0';
    return;
  }

  for (uint8_t i = 0; i < PEER_SERIAL_LEN - 1; i++) {
    buffer[i] = EEPROM.read(EEPROM_PEER_SERIAL_ADDR + i);
    if (buffer[i] == '\0' || buffer[i] == 0xFF) { // handle empty EEPROM
      buffer[i] = '\0';
      break;
    }
  }
  buffer[PEER_SERIAL_LEN - 1] = '\0';
}
