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

// Pinos
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

// Sensores
Adafruit_SGP30 sgp;
DHT dht(DHT_OUT, DHT_TYPE);
GUVAS12SD guva(GUVA_OUT, 3.3);
MQ131Class mq131(MQ_ANALOG, MQ_HEATER);
MICS4514 mics(MICS_RED, MICS_NOX, MICS_PRE);
PMS5003_ESP pms(Serial2, PMS_RXD2, PMS_TXD2);

// Tempo
unsigned long lastPMSUpdate = 0;
unsigned long lastCycle = 0;
const unsigned long PMS_INTERVAL = 1000;
const unsigned long CYCLE_INTERVAL = 10000;

// Acumuladores PMS
float sumPM1 = 0;
float sumPM25 = 0;
float sumPM10 = 0;
int pmsCount = 0;

// Funções auxiliares
uint32_t getAbsoluteHumidity(float temperature, float humidity);
float readMax(float m, float b);

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando sensores e pré-aquecimento...");

  Wire.begin(SGP_SDA, SGP_SCL);
  dht.begin();
  sgp.begin();
  sgp.IAQinit();
  mics.preHeat();
  mq131.begin();
  pms.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi!");
  delay(90000); // pré-aquecimento
}

void loop() {
  unsigned long now = millis();

  // Atualiza o PMS5003 a cada 1s e acumula leitura
  if (now - lastPMSUpdate >= PMS_INTERVAL) {
    pms.update();
    lastPMSUpdate = now;

    sumPM1 += pms.readPM1_0();
    sumPM25 += pms.readPM2_5();
    sumPM10 += pms.readPM10();
    pmsCount++;
  }

  // Executa ciclo completo a cada 10s
  if (now - lastCycle >= CYCLE_INTERVAL) {
    lastCycle = now;

    // Leitura dos sensores
    float TEMP = dht.readTemperature();
    float HUMID = dht.readHumidity();

    mq131.setEnv((int8_t)round(TEMP), (uint8_t)round(HUMID));
    float O3 = mq131.getO3(PPB);

    float DB = readMax(1.9097, -187.0903);

    sgp.setHumidity(getAbsoluteHumidity(TEMP, HUMID));
    float VOC = 0;
    float CO2 = 0;
    if (sgp.IAQmeasure()) {
      VOC = sgp.TVOC;
      CO2 = sgp.eCO2;
    }

    float CO = mics.readCO();
    float NO2 = mics.readNO2();
    float UV  = guva.index();

    // Média do PMS durante o ciclo
    float PM1 = (pmsCount > 0) ? (sumPM1 / pmsCount) : 0;
    float PM25 = (pmsCount > 0) ? (sumPM25 / pmsCount) : 0;
    float PM10 = (pmsCount > 0) ? (sumPM10 / pmsCount) : 0;

    // Reset acumuladores
    sumPM1 = 0;
    sumPM25 = 0;
    sumPM10 = 0;
    pmsCount = 0;

    // Serial Debug
    Serial.println("\n=== Leitura do Ciclo ===");
    Serial.printf("Temp: %.2f C | Umid: %.2f %%\n", TEMP, HUMID);
    Serial.printf("TVOC: %.2f ppb | CO2: %.2f ppm\n", VOC, CO2);
    Serial.printf("CO: %.2f µg/m³ | NO2: %.2f µg/m³ | O3: %.2f ppb\n", CO, NO2, O3);
    Serial.printf("PM1.0: %.2f | PM2.5: %.2f | PM10: %.2f\n", PM1, PM25, PM10);
    Serial.printf("UV Index: %.2f | Ruído: %.2f dB\n", UV, DB);

    // Enviar para servidor
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
      doc["pm1"] = roundf(PM1 * 100) / 100.0f;
      doc["pm25"] = roundf(PM25 * 100) / 100.0f;
      doc["pm10"] = roundf(PM10 * 100) / 100.0f;
      doc["uv"] = roundf(UV * 100) / 100.0f;
      doc["temperatura"] = roundf(TEMP * 100) / 100.0f;
      doc["umidade"] = roundf(HUMID * 100) / 100.0f;
      doc["ruido"] = roundf(DB * 100) / 100.0f;

      String jsonPayload;
      serializeJson(doc, jsonPayload);
      int httpCode = http.POST(jsonPayload);

      if (httpCode > 0) {
        Serial.printf("POST enviado! Código: %d\n", httpCode);
        Serial.println(http.getString());
      } else {
        Serial.printf("Erro HTTP: %d\n", httpCode);
      }
      http.end();
    }
  }
}

// === Funções auxiliares ===

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
  if (isnan(temperature) || isnan(humidity)) return 0;

  float absHumidity = 216.7f * (humidity / 100.0f) * 6.112f *
                      exp((17.62f * temperature) / (243.12f + temperature)) /
                      (273.15f + temperature);

  return (uint32_t)(absHumidity * 256.0f); // formato fixo 8.8
}

float readMax(float m, float b) {
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095;

  while (millis() - startMillis < MAX_SAMPLEWINDOW) {
    int sample = analogRead(MAX_OUT);
    if (sample < 4095) {
      if (sample > signalMax) signalMax = sample;
      if (sample < signalMin) signalMin = sample;
    }
  }

  double volts = (signalMax - signalMin) * 3.3 / 4095.0;
  double voltsRMS = volts * 0.5 * 0.707;
  double dB = 20.0 * log10(voltsRMS / 0.00631) + 94.0;
  return (dB * m) + b;
}