#ifndef MICS4514_H
#define MICS4514_H

class MICS4514 {
public:
    MICS4514(int redPin, int noxPin, int preheatPin);

    void preHeat();   // Função para pré-aquecimento do sensor
    float readCO();   // Função para ler a concentração de CO (ppm)
    float readNO2();  // Função para ler a concentração de NO2 (ppm)

private:
    int _redPin;
    int _noxPin;
    int _preheatPin;

    // Parâmetros do sensor
    const float R1_RED = 5470.0;
    const float R2_RED = 9950.0;
    const float R1_NOX = 5480.0;
    const float R2_NOX = 9880.0;
    const float V_SUPPLY = 5.16;

    const float RLOAD_RED = 46500.0;
    const float RLOAD_NOX = 21800.0;

    const float R0_RED = 56873.95;
    const float R0_NOX = 120082.80;

    // Funções auxiliares privadas (se precisar)
};

#endif
