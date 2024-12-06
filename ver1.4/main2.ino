#include <Arduino.h>
#include "Arduino_BMI270_BMM150.h"
#include "Wire.h"
#include "Filter.h"
#include "Bluetooth.h"
#include "MadgwickAHRS.h"
#include "MAX30102.h"

#include "HeartRateSpO2.h"


MAX30102 particleSensor;
HeartRateSpO2 hrSpO2Calculator;

float ax, ay, az; // 加速度计
float gx, gy, gz; // 陀螺仪
float mx, my, mz; // 磁力计

float roll = 0, pitch = 0, yaw = 0;

float dt = 0.01;    // s^(-1)
float alpha = 0.98; // 权重参数

Madgwick filter;

void setup() {
  filter.begin(200);

  Serial.begin(115200);
  unsigned long startTime = millis();
  while (!Serial) {
    if (millis() - startTime > 3000) {
      break;
    }
  }

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  Serial.println("IMU initialized");

  initFilter();

  Wire.begin();
  delay(1000);

  i2cScanner();

  if (!particleSensor.begin()) {
    Serial.println("MAX30102 not found!");
    while (1);
  }
  Serial.println("MAX30102 initialized!");

  particleSensor.setMode(MAX30102::MODE_SPO2);
  particleSensor.setADCRange(MAX30102::ADC_RANGE_16384NA);
  particleSensor.setSamplingRate(MAX30102::SAMPLING_RATE_100SPS);
  particleSensor.setResolution(MAX30102::RESOLUTION_18BIT_4110US);
  particleSensor.setLedCurrent(MAX30102::LED_RED, 60);
  particleSensor.setLedCurrent(MAX30102::LED_IR, 60);

  // 初始化心率和血氧计算类
  hrSpO2Calculator.begin(particleSensor, 100.0); // 采样率 100Hz

  initBLE();
}

void loop() {
  pollBLE();
  if (central && central.connected()) {
    readAndFuseIMUData();

    // 调用心率和血氧更新函数
    if (hrSpO2Calculator.update()) {
      float heartRate = hrSpO2Calculator.getHeartRate();
      float spo2 = hrSpO2Calculator.getSpO2();

      updateBLECharacteristics(roll_normalized, pitch_normalized, yaw_normalized, heartRate, spo2);
      printSensorData(heartRate, spo2);
    } else {
      updateBLECharacteristics(roll_normalized, pitch_normalized, yaw_normalized, 0.0, 0.0);
      printSensorData(0.0, 0.0);
    }
  }
  delay(dt * 1000);
}

// 其他函数保持不变...

void printSensorData(float heartRate, float spo2) {
  Serial.print("Normalized Roll=");
  Serial.print(roll_normalized, 4);
  Serial.print(", Normalized Pitch=");
  Serial.print(pitch_normalized, 4);
  Serial.print(", Normalized Yaw=");
  Serial.println(yaw_normalized, 4);
  Serial.println("---------------------------");

  Serial.print("Heartbeat (bpm): ");
  Serial.print(heartRate, 2);
  Serial.print(", SpO2 (%): ");
  Serial.println(spo2, 2);

  Serial.println("---------------------------");
}
