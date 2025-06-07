/*
  PM2.5 Demo
  pm25-demo.ino
  Demonstrates operation of PM2.5 Particulate Matter Sensor
  ESP32 Serial Port (RX = 16, TX = 17)
  Derived from howtoelectronics.com - https://how2electronics.com/interfacing-pms5003-air-quality-sensor-arduino/
 
  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/
#include <PMS5003.h>
// Serial Port connections for PM2.5 Sensor
#define RXD2 18  // To sensor TXD
#define TXD2 17  // To sensor RXD

void setup() {
  // our debugging output
  Serial.begin(115200);
  while (!Serial) {
    delay(10); 
  }
  // Set up UART connection
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

}



PMS5003Data data;

void loop() {
  if (data.readFromStream(&Serial1)) {
    data.print(Serial);
  }
}

