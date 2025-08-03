
#include <SX126x.h>

SX126x LoRa;


// Message to transmit
char TxMessage[] = "#BIGFISH";
uint8_t MotStaValue = 0;

uint8_t staRxMsg = 0;

void loraInit() {
  //Serial.println("Begin LoRa radio");
  int8_t nssPin = PIN_NSS, resetPin = PIN_NRST, busyPin = PIN_BUSY, irqPin = PIN_DIO1, txenPin = -1, rxenPin = -1;
  if (!LoRa.begin(nssPin, resetPin, busyPin, irqPin, txenPin, rxenPin)) {
    //Serial.println("Something wrong, can't begin LoRa radio");
    while (1);
  }
  // Uncomment below to use non default SPI port
  //SPIClass SPI_2(PB15, PB14, PB13);
  //LoRa.setSPI(SPI_2, 16000000);

  // Begin LoRa radio and set NSS, reset, busy, IRQ, txen, and rxen pin with connected arduino pins
  // IRQ pin not used in this example (set to -1). Set txen and rxen pin to -1 if RF module doesn't have one
  //Serial.println("Begin LoRa radio");
  //int8_t nssPin = PIN_NSS, resetPin = PIN_NRST, busyPin = PIN_BUSY, irqPin = PIN_DIO1, txenPin = -1, rxenPin = -1;
  if (!LoRa.begin(nssPin, resetPin, busyPin, irqPin, txenPin, rxenPin)) {
    //Serial.println("Something wrong, can't begin LoRa radio");
    while (1);
  }

  // Optionally configure TCXO or XTAL used in RF module
  // Different RF module can have different clock, so make sure clock source is configured correctly
  // uncomment code below to use TCXO
  ////Serial.println("Set RF module to use TCXO as clock reference");
  uint8_t dio3Voltage = SX126X_DIO3_OUTPUT_1_8;
  uint32_t tcxoDelay = SX126X_TCXO_DELAY_10;
  LoRa.setDio3TcxoCtrl(dio3Voltage, tcxoDelay);
  // uncomment code below to use XTAL
  //uint8_t xtalA = 0x12;
  //uint8_t xtalB = 0x12;
  ////Serial.println("Set RF module to use XTAL as clock reference");
  //LoRa.setXtalCap(xtalA, xtalB);

  // Optionally configure DIO2 as RF switch control
  // This is usually used for a LoRa module without TXEN and RXEN pins
  //LoRa.setDio2RfSwitch(true);

  // Set frequency to 915 Mhz
  //Serial.println("Set frequency to 915 Mhz");
  LoRa.setFrequency(433000000);

  // Set TX power, default power for SX1262 and SX1268 are +22 dBm and for SX1261 is +14 dBm
  // This function will set PA config with optimal setting for requested TX power
  //Serial.println("Set TX power to +17 dBm");
  LoRa.setTxPower(22, SX126X_TX_POWER_SX1262);                        // TX power +17 dBm for SX1262

  // Configure modulation parameter including spreading factor (SF), bandwidth (BW), and coding rate (CR)
  // Receiver must have same SF and BW setting with transmitter to be able to receive LoRa packet
  //Serial.println("Set modulation parameters:\n\tSpreading factor = 7\n\tBandwidth = 125 kHz\n\tCoding rate = 4/5");
  uint8_t sf = 7;                                                     // LoRa spreading factor: 7
  uint32_t bw = 125000;                                               // Bandwidth: 125 kHz
  uint8_t cr = 5;                                                     // Coding rate: 4/5
  LoRa.setLoRaModulation(sf, bw, cr);

  // Configure packet parameter including header type, preamble length, payload length, and CRC type
  // The explicit packet includes header contain CR, number of byte, and CRC type
  // Receiver can receive packet with different CR and packet parameters in explicit header mode
  //Serial.println("Set packet parameters:\n\tExplicit header type\n\tPreamble length = 12\n\tPayload Length = 15\n\tCRC on");
  uint8_t headerType = SX126X_HEADER_EXPLICIT;                        // Explicit header mode
  uint16_t preambleLength = 12;                                       // Set preamble length to 12
  uint8_t payloadLength = 15;                                         // Initialize payloadLength to 15
  bool crcType = true;                                                // Set CRC enable
  LoRa.setLoRaPacket(headerType, preambleLength, payloadLength, crcType);

  // Set syncronize word for public network (0x3444)
  //Serial.println("Set syncronize word to 0x3444");
  LoRa.setSyncWord(0x3444);

  //Serial.println("\n-- LORA TRANSMITTER --\n");
}


void loraTxFunc(char* msg, int MotStaValue) {
  // Transmit message and counter
  // write() method must be placed between beginPacket() and endPacket()
  //uint8_t nBytes = strlen(msg);
  size_t nBytes = strlen(msg);
  LoRa.beginPacket();
  LoRa.write(msg, nBytes);
  LoRa.write(MotStaValue);
  LoRa.endPacket();

  // Print message and counter in //Serial
  //Serial.print(message);
  //Serial.print("  ");
  //Serial.println(counter++);

  // Wait until modulation process for transmitting packet finish
  LoRa.wait();

  //  // Print transmit time
  //  //Serial.print("Transmit time: ");
  //  //Serial.print(LoRa.transmitTime());
  //  //Serial.println(" ms");
  //  //Serial.println();
  //// Don't load RF module with continous transmit
}



void loraRxFunc() {

  // Request for receiving new LoRa packet
  //LoRa.request();
  // Wait for incoming LoRa packet
  //LoRa.wait();

  uint32_t rxPeriod = 100;
  uint32_t sleepPeriod = 5;
  LoRa.listen(rxPeriod, sleepPeriod);

  // Put received packet to message and counter variable
  // read() and available() method must be called after request() or listen() method
  const uint8_t msgLen = LoRa.available() - 1;
  char message[msgLen];
  uint8_t counter;
  // available() method return remaining received payload length and will decrement each read() or get() method called
  uint8_t i = 0;
  while (LoRa.available() > 1) {
    message[i++] = LoRa.read();
  }
  staRxMsg = LoRa.read();

  // Print received message and counter in //Serial
  //Serial.print(message);
  //Serial.print("  ");
  //Serial.println(staRxMsg);
  String strMessage = String(message);
  if (!strMessage.indexOf("ACK_M1") && staRxMsg == 1) {
    //Serial.println(" MOTOR 1 ON - LORA ");
    funcLedReset();
    funcM1LGreen();
    buzBeep(500);
  }
  if (!strMessage.indexOf("ACK_M1") && staRxMsg == 0) {
    //Serial.println(" MOTOR 1 OFF - LORA ");
    funcLedReset();
    funcM1LRed();
    buzBeep(500);
  }

  // Print packet/signal status including package RSSI and SNR
  //Serial.print("Packet status: RSSI = ");
  //Serial.print(LoRa.packetRssi());
  //Serial.print(" dBm | SNR = ");
  //Serial.print(LoRa.snr());
  //Serial.println(" dB");

  // Show received status in case CRC or header error occur
  //  uint8_t status = LoRa.status();
  //  if (status == SX126X_STATUS_CRC_ERR) //Serial.println("CRC error");
  //  else if (status == SX126X_STATUS_HEADER_ERR) //Serial.println("Packet header error");
  //Serial.println();
}

void loraStbTx() {
  // Request for receiving new LoRa packets
  static uint32_t lastUpdate = 0;
  uint32_t now = millis();
  if (now - lastUpdate >= 5000) {
    loraTxFunc("STB", 1);
    lastUpdate = now;
  }
}
