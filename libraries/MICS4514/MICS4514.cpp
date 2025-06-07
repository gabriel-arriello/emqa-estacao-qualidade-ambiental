#include "MICS4514.h"
#include <Arduino.h>
#include <math.h>

MICS4514::MICS4514(int redPin, int noxPin, int preheatPin) {
    _redPin = redPin;
    _noxPin = noxPin;
    _preheatPin = preheatPin;

    pinMode(_preheatPin, OUTPUT);
}

void MICS4514::preHeat() {
    pinMode(_preheatPin, OUTPUT);

    // Liga aquecedor
    digitalWrite(_preheatPin, HIGH); 
}

float MICS4514::readCO() {
    float vRED_raw = analogRead(_redPin);

    // ESP32: resolução de 12 bits = 4095
    float vRED = (vRED_raw / 4095.0) * 3.3;

    // Corrigir divisor de tensão
    float vRED_sensor = vRED * ((R1_RED + R2_RED) / R2_RED);

    // Calcular Rs
    float rs_red = RLOAD_RED * ((V_SUPPLY - vRED_sensor) / vRED_sensor);

    // Calcular Rs/R0
    float ratio_red = rs_red / R0_RED;

    // Estimar ppm
    float ppm_CO = pow(10, (log10(ratio_red) - 0.556) / -0.852);

    return ppm_CO;
}

float MICS4514::readNO2() {
    float vNOX_raw = analogRead(_noxPin);

    // ESP32: resolução de 12 bits = 4095
    float vNOX = (vNOX_raw / 4095.0) * 3.3;

    // Corrigir divisor de tensão
    float vNOX_sensor = vNOX * ((R1_NOX + R2_NOX) / R2_NOX);

    // Calcular Rs
    float rs_nox = RLOAD_NOX * ((V_SUPPLY - vNOX_sensor) / vNOX_sensor);

    // Calcular Rs/R0
    float ratio_nox = rs_nox / R0_NOX;

    // Estimar ppm
    float ppm_NO2 = pow(10, (log10(ratio_nox) - 0.829) / 0.992);

    return ppm_NO2;
}
