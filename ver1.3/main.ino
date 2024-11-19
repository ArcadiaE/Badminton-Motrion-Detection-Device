
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

#include "MadgwickAHRS.h"

 #include "MAX30102.h"
//  #include "heartRate.h"

// MAX30102
MAX30102 particleSensor;

float ax, ay, az; // 加速度计
float gx, gy, gz; // 陀螺仪
float mx, my, mz; // 磁力计

float roll = 0, pitch = 0;

float dt = 0.01;    // s^(-1)
float alpha = 0.98; // 

// long irValue, redValue;
float heartRate = 0;
float spo2 = 0;

// 滤波器缓冲区大小
#define HR_FILTER_SIZE 5
float hrBuffer[HR_FILTER_SIZE];
int hrIndex = 0;
#define SPO2_FILTER_SIZE 5
float spo2Buffer[SPO2_FILTER_SIZE];
int spo2Index = 0;

Madgwick filter;

void setup() {

  filter.begin(200);

  Serial.begin(115200);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  Serial.println("IMU initialized");

  initFilter();

  // if (!particleSensor.begin(Wire) ) {
    // Serial.println("MAX30102 not found！");
    // while (1);
  // }
  // Serial.println("MAX30102 initialized!");

  // // MAX30102参数
  // byte ledBrightness = 60; // LED亮度，0-255
  // byte sampleAverage = 4;  // 采样平均次数
  // byte ledMode = 2;        // LED模式，2 = 红外和红光
  // int sampleRate = 100;    // 采样率
  // int pulseWidth = 411;    // 脉冲宽度
  // int adcRange = 16384;    // ADC范围
  
  if (!particleSensor.begin()) {
    Serial.println("MAX30102 not found!");
    while (1);
  }
  Serial.println("MAX30102 initialized!");

  particleSensor.setMode(MAX30102::MODE_SPO2); // 设置为 SPO2 模式
  particleSensor.setADCRange(MAX30102::ADC_RANGE_16384NA); // 设置 ADC 范围
  particleSensor.setSamplingRate(MAX30102::SAMPLING_RATE_100SPS); // 设置采样率为 100Hz
  particleSensor.setResolution(MAX30102::RESOLUTION_18BIT_4110US); // 设置分辨率和脉冲宽度
  particleSensor.setLedCurrent(MAX30102::LED_RED, 60); // 设置红光 LED 电流（0-255，对应 0-51mA，每步 0.2mA）
  particleSensor.setLedCurrent(MAX30102::LED_IR, 60); // 设置红外光 LED 电流

//  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

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
    // updateBLECharacteristics(roll_normalized, pitch_normalized);
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
  if (!isValidIMUData(ax, ay, az, gx, gy, gz, mx, my, mz)) {
    Serial.println("Not valid data, loading failed!");
    return;
  }


  float accThreshold = 1.0; // 根据需要调整阈值
  if (abs(ax) < accThreshold && abs(ay) < accThreshold && abs(az) < accThreshold) {
    // 如果所有轴的加速度都小于阈值，则不处理
    return;
  }

  float gyroThreshold = 0.1;
  if (abs(gx) < gyroThreshold && abs(gy) < gyroThreshold && abs(gz) < gyroThreshold) {
  // 如果陀螺仪变化小于阈值，则不处理
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
// void readAndFilterHeartRateAndSpO2() {
   // while (particleSensor.available()) {
     // redValue = particleSensor.getRed();
     // irValue = particleSensor.getIR();

     // particleSensor.nextSample();

     // float hr = 0, spo2_calc = 0;
     // bool result = calculateHeartRateAndSpO2(irValue, redValue, &hr, &spo2_calc);

     // if (result) {

       // hrBuffer[hrIndex % HR_FILTER_SIZE] = hr;
       // hrIndex++;
       // heartRate = calculateAverage(hrBuffer, HR_FILTER_SIZE);

       // spo2Buffer[spo2Index % SPO2_FILTER_SIZE] = spo2_calc;
       // spo2Index++;
       // spo2 = calculateAverage(spo2Buffer, SPO2_FILTER_SIZE);
     // }
   // }
 // }
 
 void readAndFilterHeartRateAndSpO2() {
  MAX30102Sample sample = particleSensor.readSample();

  if (sample.valid) {
    uint32_t redValue = sample.red;
    uint32_t irValue = sample.ir;

    if (irValue < 50000 || redValue < 50000) {
      return;
    }

    float hr = 0;
    float spo2_calc = 0;
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


 // bool calculateHeartRateAndSpO2(long irValue, long redValue, float *hr, float *spo2_calc) {
   // static uint32_t lastBeat = 0;
   // static float beatsPerMinute;
   // static int beatAvg = 0;

   // if (checkForBeat(irValue)) {
     // uint32_t delta = millis() - lastBeat;
     // lastBeat = millis();

     // beatsPerMinute = 60 / (delta / 1000.0);

     // static float bpmSum = 0;
     // static int bpmCount = 0;

     // bpmSum += beatsPerMinute;
     // bpmCount++;

     // beatAvg = bpmSum / bpmCount;

     // *hr = beatAvg;

     // *spo2_calc = 98.0;
     // return true;
   // } else {
     // return false;
   // }
 // }
 
 bool calculateHeartRateAndSpO2(uint32_t irValue, uint32_t redValue, float *hr, float *spo2_calc) {
  static uint32_t irBuffer[100];
  static int bufferIndex = 0;

  irBuffer[bufferIndex++] = irValue;
  if (bufferIndex == 100) {
    bufferIndex = 0;

    // 简单的峰值检测算法
    int beatCount = 0;
    for (int i = 1; i < 99; i++) {
      if (irBuffer[i] > irBuffer[i - 1] && irBuffer[i] > irBuffer[i + 1] && irBuffer[i] > 50000) {
        beatCount++;
      }
    }
    float beatsPerMinute = beatCount * 60 / (100 * 0.01); // 100 个样本，采样率 100Hz

    *hr = beatsPerMinute;

    // 简单估计 SpO2
    float R = (float)redValue / (float)irValue;
    *spo2_calc = 110.0 - 25.0 * R;
    *spo2_calc = constrain(*spo2_calc, 0, 100);

    return true;
  }

  return false;
}


 // bool checkForBeat(long irValue) {
   // return particleSensor.checkForBeat(irValue);
 // }

void printSensorData() {

  Serial.print("加速度计 (ax, ay, az): ");
  Serial.print(ax, 6); Serial.print(", ");
  Serial.print(ay, 6); Serial.print(", ");
  Serial.println(az, 6);
  Serial.println("---------------------------");

  Serial.print("陀螺仪 (gx, gy, gz): ");
  Serial.print(gx, 6); Serial.print(", ");
  Serial.print(gy, 6); Serial.print(", ");
  Serial.println(gz, 6);
  Serial.println("---------------------------");

  Serial.print("磁力计 (mx, my, mz): ");
  Serial.print(mx, 6); Serial.print(", ");
  Serial.print(my, 6); Serial.print(", ");
  Serial.println(mz, 6);
  Serial.println("---------------------------");

  Serial.print("Normalised Roll=");
  Serial.print(roll_normalized, 4);
  Serial.print(", Normalised Pitch=");
  Serial.println(pitch_normalized, 4);
  Serial.println("---------------------------");

   Serial.print("Heartbeat (bpm): ");
   Serial.print(heartRate, 2);
   Serial.print(", SpO2 (%): ");
   Serial.println(spo2, 2);

  Serial.println("---------------------------");
  // Serial.println("Copyright: Arcadia_Ebendie / Yike Zhang");
  // Serial.println("---------------------------");

}
