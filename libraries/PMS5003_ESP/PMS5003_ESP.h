#ifndef PMS5003_ESP_H
#define PMS5003_ESP_H

#include <Arduino.h>

class PMS5003_ESP {
public:
  PMS5003_ESP(HardwareSerial& serialPort, uint8_t rxPin, uint8_t txPin);

  void begin();
  void update();  // Faz a leitura e parsing se dados disponíveis

  uint16_t readPM1_0() const;
  uint16_t readPM2_5() const;
  uint16_t readPM10()  const;

private:
  HardwareSerial& _serial;
  uint8_t _rxPin;
  uint8_t _txPin;

  uint16_t _pm1_0;
  uint16_t _pm2_5;
  uint16_t _pm10;

  void processFrame(uint8_t* buffer);
  bool isValidFrame(const uint8_t* buffer);
};

#endif