#ifndef _MQ131_H_
#define _MQ131_H_
#include <Arduino.h>

enum MQ131Unit {PPM, PPB, MG_M3, UG_M3};

class MQ131Class {
	public:
    // Constructor
    MQ131Class(int analogPin, int preHeatPin);

    virtual ~MQ131Class();
	
	void begin();	// Initialize the driver
	float getO3(MQ131Unit unit);	// Read the concentration of gas
	void setEnv(int8_t tempCels, uint8_t humPc);	// Define environment
	void calibrate();	// Launch full calibration cycle and print R0

private:
	// Internal helpers

	int _preHeatPin;
	int _analogPin;

	static const float V_SUPPLY;
	static const float RLOAD_MQ;
	static const float R0_MQ;

	// Parameters for environment
	int8_t temperatureCelsius = 20;
	uint8_t humidityPercent = 65;

	// Internal function to manage the heater
	void startHeater();
	void stopHeater();

	// Get environmental correction to apply on ration Rs/R0
	float getEnvCorrectRatio();

	// Lê o Rs
	float readRs();

	// Convert gas unit of gas concentration
	float convert(float input, MQ131Unit unitIn, MQ131Unit unitOut);

	// Serial console for the debug
	Stream* debugStream = NULL;
	bool enableDebug = false;
};

extern MQ131Class MQ131;

#endif // _MQ131_H_