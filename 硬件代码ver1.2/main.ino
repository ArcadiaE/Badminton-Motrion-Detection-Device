
// 适度编程益脑，过度编程伤身
// 合理安排时间，享受健康生活

// Copyright (c) 2024 Yike Zhang (Arcadia_Ebendie)
// All rights reserved.

// This software is the confidential and proprietary information of Yike Zhang
// ("Confidential Information"). You shall not disclose such Confidential Information
// and shall use it only in accordance with the terms of the license agreement you entered into
// with Yike Zhang.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions, and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions, and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// 3. Neither the name of Yike Zhang nor the names of its contributors
//   may be used to endorse or promote products derived from this software
//   without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//     .::-------------------------------:---------==-=-     
//     ::-=+++++++++*+++++++++=++++=++++*+++++++++***#*##-    
//    .:-==++++=+=+**++++++++++++++-+++++*+++++******####*.   
//   .:-=+=++++++++*+++++++++++++++::++++*++++*+*****#####+   
//   ::-=++++++++++*+++++++++++++++:.=+++*++++********#####-  
//   :-=+++++++++++*+++++++++++++++:..++**************#####*  
//  .:-=++++++++++++++++*****++++++...-+++*+**+*******######: 
//  ::-+++++++++++++++++++++++++++=....++**+++++******######- 
//  ::=+++++++++++++++++*+++++++++=....=+++=+++++*****######= 
//  ::=+++++++++++++++++++++++++++-....:+++-++++=+*****#####+ 
//  ::+++++++++++++++++*++++++++++:.....+++.++++-+*****#####+ 
//  ::++++++++++++++++++++++*+++++......=+*.=***+******#####+ 
//  ::=++++++++++++++++++++++*+++=......-*#*###**%#****#####- 
//  .:=+++++++++++++*##########*+-......=+%%***=:+#%#**#####. 
//  .:-++++++++++++##*+*++++**+*+:........=%%#*#. .--*#####:  
//   :-+++++++++++##++==+##+#*++=........-*#%#:+. ..-######   
//   ::++++++++++++*++=#+#%+-*............-:-::: ...=######   
//   :-++*++++++++++-:.+:::.:: ............:........*######=  
//  :-++*++*+++++++=.. .:..........................-########+ 
//  -=++*++******++=...............................+######### 
//  =+++*++*+*******:.............................-########## 
//  ++++*++*+++++***:............................-+########## 
//  ++++**+*+++++*++-...........................::-#########* 
//  *+=+**+***+++*+++=...........................-*######%##- 
//  *%==+**********+***:.......................:+###**##%@%@+ 
//  -%%=-+*+**+*********+-...................:+###***#%@@@@@+ 
//  +*%@#+=++++++**********=:..............:+###%%%@@@@@@@@@+ 
//  *@@@@@%-=+++++#++=+**#*#**::.........-*#%%%@@@@@@@@@@@@@= 
//  *@@@@@@--+++++*++=-=*%##***::::::..:---+%%%%@@@@@@@@@@@@= 

#include "Arduino_BMI270_BMM150.h" 
#include "Wire.h"                   
#include "Filter.h"                 
#include "Bluetooth.h"              

#include "MAX30105.h"
#include "heartRate.h"

// MAX30105
MAX30105 particleSensor;

float ax, ay, az; // 加速度计
float gx, gy, gz; // 陀螺仪
float mx, my, mz; // 磁力计

float roll, pitch;

float dt = 0.01;    // s^(-1)
float alpha = 0.98; // 

long irValue, redValue;
float heartRate = 0;
float spo2 = 0;

// 滤波器缓冲区大小
#define HR_FILTER_SIZE 5
float hrBuffer[HR_FILTER_SIZE];
int hrIndex = 0;
#define SPO2_FILTER_SIZE 5
float spo2Buffer[SPO2_FILTER_SIZE];
int spo2Index = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  Serial.println("IMU initialized");

  initFilter();

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found！");
    while (1);
  }
  Serial.println("MAX30102 initialized!");

  // MAX30102参数
  byte ledBrightness = 60; // LED亮度，0-255
  byte sampleAverage = 4;  // 采样平均次数
  byte ledMode = 2;        // LED模式，2 = 红外和红光
  int sampleRate = 100;    // 采样率
  int pulseWidth = 411;    // 脉冲宽度
  int adcRange = 16384;    // ADC范围

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  for (int i = 0; i < HR_FILTER_SIZE; i++) {
    hrBuffer[i] = 0;
  }
  for (int i = 0; i < SPO2_FILTER_SIZE; i++) {
    spo2Buffer[i] = 0;
  }

  initBLE();
}

void loop() {
  pollBLE();
  if (central && central.connected()) {
    readAndFuseIMUData();
    readAndFilterHeartRateAndSpO2();
    updateBLECharacteristics(roll_normalized, pitch_normalized, heartRate, spo2);
    printSensorData();
  }
  delay(dt * 1000); //Freq.
}

void readAndFuseIMUData() {
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
  } 
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx, gy, gz);
  }
  if (IMU.magneticFieldAvailable()) {
    IMU.readMagneticField(mx, my, mz);
  }
  if (!isValidIMUData(ax, ay, az, gx, gy, gz)) {
    Serial.println("Not valid data, loading failed!");
    return;
  }

  imuLowPassFilter(ax, ay, az, gx, gy, gz); //LPF

  float rollAcc = atan2(ay_filtered, az_filtered) * 180 / PI;
  float pitchAcc = atan(-ax_filtered / sqrt(ay_filtered * ay_filtered + az_filtered * az_filtered)) * 180 / PI;

  static float rollGyro = 0;
  static float pitchGyro = 0;

  rollGyro += gx_filtered * dt;
  pitchGyro += gy_filtered * dt;

  roll = alpha * (roll + gx_filtered * dt) + (1 - alpha) * rollAcc;
  pitch = alpha * (pitch + gy_filtered * dt) + (1 - alpha) * pitchAcc;

  normalizeIMUData(roll, pitch);
}

//Heartbeat & SpO2
void readAndFilterHeartRateAndSpO2() {
  while (particleSensor.available()) {
    redValue = particleSensor.getRed();
    irValue = particleSensor.getIR();

    particleSensor.nextSample();

    float hr = 0, spo2_calc = 0;
    bool result = calculateHeartRateAndSpO2(irValue, redValue, &hr, &spo2_calc);

    if (result) {

      hrBuffer[hrIndex % HR_FILTER_SIZE] = hr;
      hrIndex++;
      heartRate = calculateAverage(hrBuffer, HR_FILTER_SIZE);

      spo2Buffer[spo2Index % SPO2_FILTER_SIZE] = spo2_calc;
      spo2Index++;
      spo2 = calculateAverage(spo2Buffer, SPO2_FILTER_SIZE);
    }
  }
}

bool calculateHeartRateAndSpO2(long irValue, long redValue, float *hr, float *spo2_calc) {
  static uint32_t lastBeat = 0;
  static float beatsPerMinute;
  static int beatAvg = 0;

  if (checkForBeat(irValue)) {
    uint32_t delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    static float bpmSum = 0;
    static int bpmCount = 0;

    bpmSum += beatsPerMinute;
    bpmCount++;

    beatAvg = bpmSum / bpmCount;

    *hr = beatAvg;

    *spo2_calc = 98.0;
    return true;
  } else {
    return false;
  }
}

bool checkForBeat(long irValue) {
  return particleSensor.checkForBeat(irValue);
}

void printSensorData() {
  Serial.print("Normalised Roll=");
  Serial.print(roll_normalized, 4);
  Serial.print(", Normalised Pitch=");
  Serial.println(pitch_normalized, 4);

  Serial.print("Heartbeat (bpm): ");
  Serial.print(heartRate, 2);
  Serial.print(", SpO2 (%): ");
  Serial.println(spo2, 2);

  Serial.println("---------------------------");
  Serial.println("Copyright: Arcadia_Ebendie / Yike Zhang");
  Serial.println("---------------------------");

}
