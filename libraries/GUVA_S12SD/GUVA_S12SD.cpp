/**
 * Copyright 2017 Kohei MATSUSHITA
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "GUVA_S12SD.h"

/* https://github.com/Marzogh/Tricorder/blob/master/Tricorder/GUVAS12SD.ino */

GUVAS12SD::GUVAS12SD(int connectedPinNo, float workingVoltage, int samplingCount)
{
    port = connectedPinNo;
    v = workingVoltage;
    s = samplingCount;

    // Configura a resolução do ADC para 12 bits (0-4095) - padrão no ESP32
    analogReadResolution(12);
    // Configura a atenuação para 11dB (range de 0-3.3V)
    analogSetAttenuation(ADC_11db);

}
float GUVAS12SD::read()
{
    /*float sum = 0;
    for(int i = 0; i < s; i++) {
        float v = analogRead(port);
        sum += v;
        delay(2);
    }*/
    //float sensor_value_average = sum / s;
    float sensor_value = analogRead(port);
    // Converte a leitura para mV (considerando que v é a tensão de referência em volts)
    float mV = ((float) sensor_value / 4095.0) * v * 1000.0;
    return mV;
}

int GUVAS12SD::index() {
    int uv, vout;
    uv = 0;
    vout = 0;
    vout = (int)read();

    if (vout < 50) {
        uv = 0;
    }
    else if (vout < 227) {
        uv = 1;
    }
    else if (vout < 318) {
        uv = 2;
    }
    else if (vout < 408) {
        uv = 3;
    }
    else if (vout < 503) {
        uv = 4;
    }
    else if (vout < 606) {
        uv = 5;
    }
    else if (vout < 696) {
        uv = 6;
    }
    else if (vout < 795) {
        uv = 7;
    }
    else if (vout < 881) {
        uv = 8;
    }
    else if (vout < 976) {
        uv = 9;
    }
    else if (vout < 1079) {
        uv = 10;
    }
    else {
        uv = 11;
    }
    return uv;
}

