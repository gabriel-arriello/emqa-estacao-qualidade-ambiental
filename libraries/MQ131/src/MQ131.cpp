#include "MQ131.h"
#include <Arduino.h>

// Definições das constantes estáticas da classe MQ131Class
const float MQ131Class::V_SUPPLY = 3.26;
const float MQ131Class::RLOAD_MQ = 980000.0;
const float MQ131Class::R0_MQ = 240452.89;

/**
 * Construtor, nada especial a fazer
 */
MQ131Class::MQ131Class(int analogPin, int preHeatPin) {
	_analogPin = analogPin;
	_preHeatPin = preHeatPin;
}

/**
 * Destrutor, nada especial a fazer
 */
MQ131Class::~MQ131Class() {
}

/**
 * Inicializa variáveis principais
 */
void MQ131Class::begin() {
	enableDebug = true;  // Altere para false se quiser desligar logs

	pinMode(_preHeatPin, OUTPUT);
	pinMode(_analogPin, INPUT);
	startHeater();
}

/**
 * Inicia o aquecedor
 */
void MQ131Class::startHeater() {
	pinMode(_preHeatPin, OUTPUT);
	digitalWrite(_preHeatPin, HIGH);
}

/**
 * Desliga o aquecedor
 */
void MQ131Class::stopHeater() {
	digitalWrite(_preHeatPin, LOW);
}

/**
 * Define valores ambientais
 */
void MQ131Class::setEnv(int8_t tempCels, uint8_t humPc) {
	temperatureCelsius = tempCels;
	humidityPercent = humPc;
}

/**
 * Obtém a correção a aplicar em Rs com base nas condições ambientais
 */
float MQ131Class::getEnvCorrectRatio() {
	if (humidityPercent == 65 && temperatureCelsius == 20) return 1.0;
	if (humidityPercent > 75) return -0.0103 * temperatureCelsius + 1.1507;
	if (humidityPercent > 50) return -0.0119 * temperatureCelsius + 1.3261;
	return -0.0141 * temperatureCelsius + 1.5623;
}

float MQ131Class::readRs() {
	float vSensor_raw = analogRead(_analogPin);
	float vSensor = (vSensor_raw / 4095.0) * 3.3;
	return RLOAD_MQ * ((V_SUPPLY - vSensor) / vSensor);
}

/**
 * Obtém a concentração de gás O3 em ppm
 */
float MQ131Class::getO3(MQ131Unit unit) {
	float rs = readRs();
	float ratio = rs / R0_MQ * getEnvCorrectRatio();
	return convert(9.4783 * pow(ratio, 2.3348), PPB, unit);
}

/**
 * Converte unidade de concentração de gás
 */
float MQ131Class::convert(float input, MQ131Unit unitIn, MQ131Unit unitOut) {
	if (unitIn == unitOut) return input;

	float concentration = 0;

	switch (unitOut) {
		case PPM: return input / 1000.0;
		case PPB: return input * 1000.0;
		case MG_M3:
			concentration = (unitIn == PPM) ? input : input / 1000.0;
			return concentration * 48.0 / 22.71108;
		case UG_M3:
			concentration = (unitIn == PPB) ? input : input * 1000.0;
			return concentration * 48.0 / 22.71108;
		default:
			return input;
	}
}

void MQ131Class::calibrate() {
	float lastRsValue = -1;
	uint8_t countStable = 0;
	uint16_t count = 0;
	float sum = 0;
	const uint8_t timeToReadConsistency = 15;
	const float stabilityThreshold = 0.025;  // 2,5% de tolerância para considerar a leitura estável

	if (enableDebug) {
		Serial.println(F("MQ131 : Iniciando calibração..."));
		Serial.println(F("MQ131 : Ligando aquecedor"));
		Serial.print(F("MQ131 : Ciclos estáveis necessários: "));
		Serial.println(timeToReadConsistency);
	}

	startHeater();

	while (countStable < timeToReadConsistency) {
		float value = readRs();

		if (enableDebug) {
			Serial.print(F("MQ131 : Rs lido = "));
			Serial.print(value, 2);
			Serial.println(F(" Ohms"));
		}

		// Considera a leitura estável se a diferença percentual for menor que o threshold
		if (lastRsValue < 0 || fabs(value - lastRsValue) / lastRsValue > stabilityThreshold) {
			lastRsValue = value;
			countStable = 0;
			sum = 0;
		} else {
			sum += value;
			countStable++;
		}

		count++;
		delay(1000);
	}

	float averageRs = sum / timeToReadConsistency;

	if (enableDebug) {
		Serial.print(F("MQ131 : Estabilização após "));
		Serial.print(count);
		Serial.println(F(" segundos"));
		Serial.println(F("MQ131 : Desligando aquecedor"));

		Serial.print(F("MQ131 : Valor médio de R0 = "));
		Serial.print(averageRs, 2);
		Serial.println(F(" Ohms"));

		Serial.println(F("MQ131 : Atualize o valor de R0_MQ manualmente no código com o valor acima."));
	}

	stopHeater();
}
