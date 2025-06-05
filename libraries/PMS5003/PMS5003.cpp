#include "PMS5003.h"
#include <cstring>    // Para memcpy

// Construtor
PMS5003Data::PMS5003Data() {}


// Método para ler os dados do stream
bool PMS5003Data::readFromStream(Stream *s) {
    if (!s->available()) return false;
    if (s->peek() != 0x42) {
      s->read();
      return false;
    }
    if (s->available() < 32) return false;

    uint8_t buffer[32];
    s->readBytes(buffer, 32);

    if (!verifyChecksum(buffer)) {
      Serial.println("Checksum failure");
      return false;
    }

    fromRawBuffer(buffer);
    return true;
  }



// Método para exibir os dados formatados
void PMS5003Data::print(Stream &out) {
  out.println();
  out.println("---------------------------------------");
  out.println("Concentration Units (standard)");
  out.print("PM 1.0: ");
  out.print(pm10_standard);
  out.print("\t\tPM 2.5: ");
  out.print(pm25_standard);
  out.print("\t\tPM 10: ");
  out.println(pm100_standard);
  out.println("---------------------------------------");
  out.println("Concentration Units (environmental)");
  out.print("PM 1.0: ");
  out.print(pm10_env);
  out.print("\t\tPM 2.5: ");
  out.print(pm25_env);
  out.print("\t\tPM 10: ");
  out.println(pm100_env);
  out.println("---------------------------------------");
  out.print("Particles > 0.3um / 0.1L air: ");
  out.println(particles_03um);
  out.print("Particles > 0.5um / 0.1L air: ");
  out.println(particles_05um);
  out.print("Particles > 1.0um / 0.1L air: ");
  out.println(particles_10um);
  out.print("Particles > 2.5um / 0.1L air: ");
  out.println(particles_25um);
  out.print("Particles > 5.0um / 0.1L air: ");
  out.println(particles_50um);
  out.print("Particles > 10.0um / 0.1L air: ");
  out.println(particles_100um);
  out.println("---------------------------------------");
}

void PMS5003Data::fromRawBuffer(const uint8_t *buffer) {
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }
  memcpy((void *)this, buffer_u16, 30);
}

bool PMS5003Data::verifyChecksum(const uint8_t *buffer) {
  uint16_t sum = 0;
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }
  uint16_t receivedChecksum;
  receivedChecksum = (buffer[30] << 8) | buffer[31];
  return sum == receivedChecksum;
}
