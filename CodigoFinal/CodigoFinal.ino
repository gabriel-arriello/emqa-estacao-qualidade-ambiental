#include <Wire.h>
#include "Adafruit_SGP30.h"
#include <DHT.h>
#include <GUVA_S12SD.h>
#include <MICS4514.h>
#include "PMS5003.h"
#include <MQ131.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "gabriel";
const char* password = "arriello";
const char* serverUMQ_RL = "http://192.168.196.197:5000/dados";

// Definições de pinos
#define MQ_ANALOG 1
#define MQ_HEATER 2
#define SGP_SDA 8
#define SGP_SCL 9
#define DHT_OUT 4
#define GUVA_OUT 3
#define MAX_OUT 10
#define MICS_RED 7
#define MICS_NOX 6
#define MICS_PRE 5
#define PMS_RXD2 18
#define PMS_TXD2 17

// Constantes
#define DHT_TYPE DHT22
#define MAX_SAMPLEWINDOW 50      // Janela de amostragem menor para mais leituras
#define SAMPLE_INTERVAL 50       // Intervalo entre amostras de áudio (ms)
#define CYCLE_TIME 10000         // Tempo total do ciclo (10 segundos)

// Variáveis globais
PMS5003Data pmsData;             // Instância da classe PMS
unsigned long lastSampleTime = 0;
unsigned long cycleStartTime = 0;
float dbSum = 0;
int dbSamplesCount = 0;

// Sensores
Adafruit_SGP30 sgp;
DHT dht(DHT_OUT, DHT_TYPE);
GUVAS12SD guva(GUVA_OUT, 3.3);
MQ131Class mq131(MQ_ANALOG, MQ_HEATER);
MICS4514 mics(MICS_RED, MICS_NOX, MICS_PRE);

// Definição das funções auxiliares
uint32_t getAbsoluteHumidity(float temperature, float humidity);
float readDBWindow();
float convertNO2_ppm_to_ugm3(float ppm_NO2, float temperature = 25.0, float pressure = 1013.25);

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando sensores e pré-aquecimento...");

  Serial2.begin(9600, SERIAL_8N1, PMS_RXD2, PMS_TXD2);
  Wire.begin(SGP_SDA, SGP_SCL);
  dht.begin();
  sgp.begin();
  sgp.IAQinit();
  mics.preHeat();
  mq131.begin();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi!");

  delay(90000); // Esperar pré-aquecimento inicial
  cycleStartTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long elapsed = currentTime - cycleStartTime;

  // Coleta contínua de dB durante todo o ciclo
  if (currentTime - lastSampleTime >= SAMPLE_INTERVAL) {
    float db = readDBWindow();
    dbSum += db;
    dbSamplesCount++;
    lastSampleTime = currentTime;
  }

  // Leitura não bloqueante do PMS5003
  if (pmsData.readFromStream(&Serial2)) {
    // Atualiza os valores se novos dados chegarem
  }

  // Final do ciclo (10 segundos)
  if (elapsed >= CYCLE_TIME) {
    // Calcular média de dB
    float avgDB = (dbSamplesCount > 0) ? dbSum / dbSamplesCount : 0;

    // Resetar variáveis de dB para o próximo ciclo
    dbSum = 0;
    dbSamplesCount = 0;
    cycleStartTime = currentTime;

    // Leitura DHT22
    float TEMP = dht.readTemperature();
    float HUMID = dht.readHumidity();

    // Leitura MQ131
    mq131.setEnv((int8_t)round(TEMP), (uint8_t)round(HUMID));
    float O3 = mq131.getO3(UG_M3);

    // Leitura SGP30
    sgp.setHumidity(getAbsoluteHumidity(TEMP, HUMID));
    float VOC = 0;
    float CO2 = 0;
    if (sgp.IAQmeasure()) {
      VOC = sgp.TVOC;
      CO2 = sgp.eCO2;
    }

    // Leitura MICS
    float CO = mics.readCO();
    float NO2 = convertNO2_ppm_to_ugm3(mics.readNO2(), TEMP);

    // Leitura GUVA
    float UV  = guva.index();

    // Leitura PMS5003 (últimos valores lidos)
    //float PM1 = pmsData.pm10_standard;
    float PM25 = pmsData.pm25_standard;
    float PM10 = pmsData.pm100_standard;

    // Exibição dos resultados
    Serial.println("\n=== Leitura do Ciclo ===");
    Serial.print("Temperatura: "); Serial.println(TEMP);
    Serial.print("Umidade: "); Serial.println(HUMID);
    Serial.print("Ruído médio (dB): "); Serial.println(avgDB);
    Serial.print("TVOC (ppb): "); Serial.println(VOC);
    Serial.print("eCO2 (ppm): "); Serial.println(CO2);
    Serial.print("CO (µg/m³): "); Serial.println(CO);
    Serial.print("NO2 (µg/m³): "); Serial.println(NO2);
    Serial.print("UV Index: "); Serial.println(UV);
    //Serial.print("PM1.0: "); Serial.println(PM1);
    Serial.print("PM2.5: "); Serial.println(PM25);
    Serial.print("PM10: "); Serial.println(PM10);
    Serial.print("O3 (µg/m³): "); Serial.println(O3);

    // Envio para servidor
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUMQ_RL);
      http.addHeader("Content-Type", "application/json");

      DynamicJsonDocument doc(512);
      doc["co"] = roundf(CO * 100) / 100.0f;
      doc["no2"] = roundf(NO2 * 100) / 100.0f;
      doc["voc"] = roundf(VOC * 100) / 100.0f;
      doc["o3"] = roundf(O3 * 100) / 100.0f;
      doc["co2"] = roundf(CO2 * 100) / 100.0f;
      //doc["pm1"] = roundf(PM1 * 100) / 100.0f;
      doc["pm25"] = roundf(PM25 * 100) / 100.0f;
      doc["pm10"] = roundf(PM10 * 100) / 100.0f;
      doc["uv"] = roundf(UV * 100) / 100.0f;
      doc["temperatura"] = roundf(TEMP * 100) / 100.0f;
      doc["umidade"] = roundf(HUMID * 100) / 100.0f;
      doc["ruido"] = roundf(avgDB * 100) / 100.0f;

      String jsonPayload;
      serializeJson(doc, jsonPayload);

      int httpResponseCode = http.POST(jsonPayload);

      if (httpResponseCode > 0) {
        Serial.print("Resposta HTTP: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
      } else {
        Serial.print("Erro no POST: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    }
  }

  // Pequeno delay para evitar sobrecarga
  delay(10);
}

// Função para ler uma janela de dB (não bloqueante por longos períodos)
float readDBWindow() {
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095; // ADC 12 bits ESP32
  const double micSensitivity = 0.00631;
  const float m = 1.9097;
  const float b = -187.0903;

  // Janela de amostragem reduzida
  while (millis() - startMillis < MAX_SAMPLEWINDOW) {
    int sample = analogRead(MAX_OUT);

    if (sample < 4095) {
      if (sample > signalMax) signalMax = sample;
      if (sample < signalMin) signalMin = sample;
    }
  }

  unsigned int peakToPeak = signalMax - signalMin;
  double volts = (peakToPeak * 3.3) / 4095.0;
  double voltsRMS = volts * 0.5 * 0.707; // Conversão pico-RMS
  double dB = 20.0 * log10(voltsRMS / micSensitivity) + 94.0;
  dB = (dB * m) + b; // Aplicar ajustes

  return dB;
}

// Função que calcula umidade absoluta
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
  if (isnan(temperature) || isnan(humidity)) return 0;

  float absHumidity = 216.7f * (humidity / 100.0f) * 6.112f *
  exp((17.62f * temperature) / (243.12f + temperature)) /
  (273.15f + temperature);

  return (uint32_t)(absHumidity * 256.0f);
}

// Função que converte o NO2 de ppm para ug/m³
float convertNO2_ppm_to_ugm3(float ppm_NO2, float temperature, float pressure) {
  // Fórmula: µg/m³ = ppm × (molecular weight / molar volume) × 1000
  // Molecular weight of NO2 = 46.0055 g/mol
  // Molar volume at 25°C and 1 atm = 24.465 L/mol

  // Ajuste para condições reais (opcional)
  float molar_volume = 24.465 * (pressure / 1013.25) * (298.15 / (temperature + 273.15));

  return ppm_NO2 * (46.0055 / molar_volume) * 1000;
}
