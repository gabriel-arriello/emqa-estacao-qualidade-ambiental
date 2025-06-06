#include <Wire.h>
#include "Adafruit_SGP30.h"
#include <DHT.h>
#include <GUVA_S12SD.h>
#include <MICS4514.h>
#include <PMS5003_ESP.h>
#include <MQ131.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "gabriel";
const char* password = "arriello";
const char* serverUMQ_RL = "http://192.168.254.197:5000/dados";

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
#define MAX_SAMPLEWINDOW 100

// Variáveis globais

// Sensores
Adafruit_SGP30 sgp;
DHT dht(DHT_OUT, DHT_TYPE);
GUVAS12SD guva(GUVA_OUT, 3.3);
MQ131Class mq131(MQ_ANALOG, MQ_HEATER);
MICS4514 mics(MICS_RED, MICS_NOX, MICS_PRE);
PMS5003_ESP pms(Serial2, PMS_RXD2, PMS_TXD2);

// Definição das funcoes auxiliares
uint32_t getAbsoluteHumidity(float temperature, float humidity);
void calibrateSGP30();
float readMicRMS();
float convertVoltageToDB(float m, float b, float voltageRMS);

void setup() {
  Serial.begin(115200);
  Serial.println("Inicialiazando sensores e pré-aquecimento...");
  Wire.begin(SGP_SDA, SGP_SCL);
  dht.begin();
  pms.begin();
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
  
  delay(120000); // Esperar pré-aquecimento inicial
}

void loop() {
  // Leitura DHT22
  float TEMP = dht.readTemperature();
  float HUMID = dht.readHumidity();

  // Leitura MQ131
  mq131.setEnv((int8_t)round(TEMP), (uint8_t)round(HUMID));
  float O3 = mq131.getO3(PPB);

  // Leitura MAX9814
  float DB = readMax(1.9097, -187.0903);

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
  float NO2 = mics.readNO2();

  // Leitura GUVA
  float UV  = guva.index();

  // Leitura PMS5003
  pms.update();
  float PM1 = pms.readPM1_0();
  float PM25 = pms.readPM2_5();
  float PM10 = pms.readPM10();
    
  // Exibição dos resultados
  Serial.println("\n=== Leitura do Ciclo ===");
  Serial.print("Temperatura: "); Serial.println(TEMP);
  Serial.print("Umidade: "); Serial.println(HUMID);
  Serial.print("Ruído (dB): "); Serial.println(DB);
  Serial.print("TVOC (ppb): "); Serial.println(VOC);
  Serial.print("eCO2 (ppm): "); Serial.println(CO2);
  Serial.print("CO (µg/m³): "); Serial.println(CO);
  Serial.print("NO2 (µg/m³): "); Serial.println(NO2);
  Serial.print("UV Index: "); Serial.println(UV);
  Serial.print("PM1.0: "); Serial.println(PM1);
  Serial.print("PM2.5: "); Serial.println(PM25);
  Serial.print("PM10: "); Serial.println(PM10);
  Serial.print("O3 (ppb): "); Serial.println(O3);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUMQ_RL);
    http.addHeader("Content-Type", "application/json");

    // Gerar json com os dados das leituras
    DynamicJsonDocument doc(512);
    doc["co"] = roundf(CO * 100) / 100.0f;
    doc["no2"] = roundf(NO2 * 100) / 100.0f;
    doc["voc"] = roundf(VOC * 100) / 100.0f;
    doc["o3"] = roundf(O3 * 100) / 100.0f;
    doc["co2"] = roundf(CO2 * 100) / 100.0f;
    doc["pm1"] = roundf(PM1 * 100) / 100.0f;
    doc["pm25"] = roundf(PM25 * 100) / 100.0f;
    doc["pm10"] = roundf(PM10 * 100) / 100.0f;
    doc["uv"] = roundf(UV * 100) / 100.0f;
    doc["temperatura"] = roundf(TEMP * 100) / 100.0f;
    doc["umidade"] = roundf(HUMID * 100) / 100.0f;
    doc["ruido"] = roundf(DB * 100) / 100.0f;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    int httMICS_PREsponseCode = http.POST(jsonPayload);
    
  if (httMICS_PREsponseCode > 0) {
    Serial.print("Resposta HTTP: ");
    Serial.println(httMICS_PREsponseCode);
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Erro no POST: ");
    Serial.println(httMICS_PREsponseCode);
  }
    
  http.end();
  }
  delay(10000);
}

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
  // Verifica se os valores são válidos
  if (isnan(temperature) || isnan(humidity)) {
    return 0;
  }

  // Fórmula da umidade absoluta (dv) em g/m³
  float absHumidity = 216.7f * (humidity / 100.0f) * 6.112f * 
                      exp((17.62f * temperature) / (243.12f + temperature)) / 
                      (273.15f + temperature);

  // Converte g/m³ para formato fixo 8.8 em mg/m³ (1 g = 1000 mg)
  // 8.8 significa: parte inteira << 8 | parte fracionária (em 256 passos)
  uint32_t fixedHumidity = (uint32_t)(absHumidity * 256.0f);

  return fixedHumidity;
}

float readMax(float m, float b){
  // Coletar sinal do microfone
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095; // ADC 12 bits ESP32
  const double micSensitivity = 0.00631;

  while (millis() - startMillis < MAX_SAMPLEWINDOW) {
    int sample = analogRead(MAX_OUT);

    if (sample < 4095) {
      if (sample > signalMax) {
        signalMax = sample;
      }
      if (sample < signalMin) {
        signalMin = sample;
      }
    }
  }

  unsigned int peakToPeak = signalMax - signalMin;

  // Converter para tensão (3.3V)
  double volts = (peakToPeak * 3.3) / 4095.0;

  // Convertendo para Volts RMS
  double voltsRMS = volts * 0.5 * 0.707;

  // Calcular dB SPL
  double dB = 20.0 * log10(voltsRMS / micSensitivity) + 94.0;
  dB = (dB * m) + b; // Aplicar ajustes

  return dB;
}
