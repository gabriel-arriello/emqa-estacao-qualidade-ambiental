#include "PMS5003_ESP.h"

PMS5003_ESP::PMS5003_ESP(HardwareSerial& serialPort, uint8_t rxPin, uint8_t txPin)
  : _serial(serialPort), _rxPin(rxPin), _txPin(txPin),
    _pm1_0(0), _pm2_5(0), _pm10(0) {}

void PMS5003_ESP::begin() {
  _serial.begin(9600, SERIAL_8N1, _rxPin, _txPin);
}

void PMS5003_ESP::update() {
  if (_serial.available()) {
    if (_serial.peek() == 0x42) {  // Verifica byte de sincronismo
      uint8_t buffer[32];
      if (_serial.readBytes(buffer, 32) == 32) {
        if (buffer[1] == 0x4D && isValidFrame(buffer)) {
          processFrame(buffer);
        }
      }
    } else {
      _serial.read();  // Descarta byte inválido
    }
  }
}

bool PMS5003_ESP::isValidFrame(const uint8_t* buffer) {
  uint16_t checksum = 0;
  for (int i = 0; i < 30; i++) {
    checksum += buffer[i];
  }
  uint16_t received = (buffer[30] << 8) | buffer[31];
  return checksum == received;
}

void PMS5003_ESP::processFrame(uint8_t* buffer) {
  _pm1_0 = (buffer[10] << 8) | buffer[11];
  _pm2_5 = (buffer[12] << 8) | buffer[13];
  _pm10  = (buffer[14] << 8) | buffer[15];
}

uint16_t PMS5003_ESP::readPM1_0() const {
  return _pm1_0;
}

uint16_t PMS5003_ESP::readPM2_5() const {
  return _pm2_5;
}

uint16_t PMS5003_ESP::readPM10() const {
  return _pm10;
}