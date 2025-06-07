#ifndef PMS5003_H
#define PMS5003_H
#include <stdint.h>   // Para uint8_t, uint16_t
#include <Arduino.h>  // Para Stream, Serial, etc.

class PMS5003Data {
public:
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um;
  uint16_t particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;

  PMS5003Data();

  bool readFromStream(Stream *s);

  void print(Stream &out);

private:
  void fromRawBuffer(const uint8_t *buffer);

  bool verifyChecksum(const uint8_t *buffer);
};
#endif
