# MQ131 Ozone Sensor Library for ESP32

This library is an adaptation of [ostaquet's Arduino-MQ131-driver](https://github.com/ostaquet/Arduino-MQ131-driver) specifically optimized for ESP32 microcontrollers, featuring hardware modifications for improved compatibility and performance.

## Key Adaptations
- **Transistor Replacement**: Uses IRLZ44N (3.3V compatible) instead of IRF840 (5V)
- **Voltage Optimization**: Adjusted working voltage parameters for ESP32's 3.3V logic
- **Precision Calibration**: Enhanced calibration routine with stability detection
- **Environmental Compensation**: Automatic temperature/humidity corrections

## Hardware Setup
### Required Components
- ESP32 development board
- MQ131 ozone sensor (low/medium/high concentration version)
- IRLZ44N MOSFET transistor
- 10KΩ resistor
- Breadboard and jumper wires

### Wiring Diagram

```plaintext
MQ131 Sensor ESP32 IRLZ44N Transistor
  VCC ────┬── 3.3V
          │
  GND ────┼── GND
          │
  Heater ─┴── Gate ─── ESP32_PREHEAT_PIN
          │
Sense ── 10KΩ ─┴── Drain
│
└─── ESP32_ANALOG_PIN
    ```


## Installation
1. Download the library as ZIP
2. In Arduino IDE:  
   `Sketch > Include Library > Add .ZIP Library...`
3. Include in your project:
   ```cpp
   #include <MQ131.h>
    ```
   
## Basic Usage

```cpp
#define PREHEAT_PIN 22   // ESP32 GPIO for heater control
#define ANALOG_PIN  34   // ESP32 analog input pin

MQ131Class MQ131(ANALOG_PIN, PREHEAT_PIN);

void setup() {
  Serial.begin(115200);
  MQ131.begin();
  MQ131.setEnv(25, 60);  // Set temperature(°C) and humidity(%)
}

void loop() {
  // Read ozone concentration in PPB
  float ozone_ppb = MQ131.getO3(PPB);
  
  Serial.print("Ozone: ");
  Serial.print(ozone_ppb);
  Serial.println(" ppb");

  delay(5000);
}

    ```
Calibration Procedure

    Power sensor in clean air environment

    Run calibration command:
```cpp
void setup() {
  // ... initialization code ...
  MQ131.calibrate();  // 15-minute stabilization
}
```

    Monitor serial output for calculated R0 value

    Update R0_MQ constant in MQ131.h with new value

Output Units

Supported measurement units:
Unit	Description
PPM	Parts per million
PPB	Parts per billion
MG_M3	Milligrams per m³
UG_M3	Micrograms per m³
Key Notes

    Pre-heating: Sensor requires 2-minute warm-up for stable readings

    Calibration: Mandatory after hardware changes or every 6 months

    Environment: Always set current temp/humidity for accurate readings

    Power: Use stable 3.3V power source (ESP32's 3.3V pin recommended)

Technical Changes from Original
Parameter	Original Value	ESP32 Adaptation
Control Transistor	IRF840 (5V)	IRLZ44N (3.3V)
Supply Voltage	5V	3.3V
V_SUPPLY	5.0	3.26
RLOAD_MQ	1,000,000 Ω	950,000 Ω
Analog Resolution	10-bit (1024)	12-bit (4096)
License

MIT License - Same as original library. See included LICENSE file.
