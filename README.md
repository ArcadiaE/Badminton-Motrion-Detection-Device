# Wearable Device for Badminton Motion Detection

## Project Overview
This project aims to design a wearable device for badminton motion detection. By using sensors and machine learning, this device analyzes real-time motion data and provides personalized exercise recommendations.

## Table of Contents
1. [Background and Objectives](#background-and-objectives)
2. [Hardware Design](#hardware-design)
3. [Software Design](#software-design)
4. [Project Implementation Steps](#project-implementation-steps)
5. [Budget and Cost Control](#budget-and-cost-control)
6. [Features and Advantages](#features-and-advantages)

## Background and Objectives
The goal of this project is to create a wearable device capable of detecting and analyzing badminton motions, with the potential to deliver personalized exercise feedback to users.

## Hardware Design
- **Core Microcontroller**: Arduino Nano 33 BLE Sense Rev2
  - 9-axis IMU sensor (accelerometer, gyroscope, magnetometer)
  - Built-in Bluetooth Low Energy (BLE) for wireless data transmission
- **Heart Rate and SpO2 Sensor**: MAX30102
  - Measures heart rate and blood oxygen levels
- **Power Supply**: LiPo Battery (3.7V, 400mAh - 1000mAh)
  - Includes a micro-USB charging module
- **Enclosure and Wearing Method**: Wristband or Velcro with 3D-printed casing

## Software Design
- **Microcontroller Programming**: 
  - Programmed using Arduino IDE for sensor data collection, processing, and BLE transmission
- **Data Processing and Machine Learning**:
  - Python environment with libraries like scikit-learn for data analysis and model training

## Project Implementation Steps
1. **Hardware Assembly**: Connect the microcontroller, sensors, and power modules, and secure the device.
2. **Software Development**: Write code for data processing and BLE communication.
3. **Data Collection and Model Training**: Collect motion data, preprocess, and train machine learning models.
4. **System Integration and Testing**: Integrate hardware and software, and conduct performance testing.
