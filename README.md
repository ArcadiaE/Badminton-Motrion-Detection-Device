# Wearable Device for Badminton Motion Detection

This repository contains the source code and documentation for the project "Wearable Device for Badminton Motion Detection". This project aims to design and implement a wearable device focused on real-time detection of badminton stroke movements. 

The device can collect motion and physiological data, which are transmitted to a mobile device for analysis via a Bluetooth Low Energy (BLE) module. The system utilizes a Support Vector Machine (SVM) to classify badminton strokes (including forehand, backhand, and smash), achieving an accuracy of 77.37%. Additionally, the system was able to generate real-time, personalised coaching feedback through an integrated web interface using the ChatGPT API.

## Hardware Architecture
The system is custom-encapsulated in the form of a wristwatch.
* **Microcontroller:** Arduino Nano 33 BLE Sense Rev2.
* **IMU:** LSM9DS1 nine-axis inertial measurement unit.
* **Biosensor:** MAX30102 biosensor module for measuring the user's heart rate and blood oxygen saturation.
* **Power Supply:** 3.7v, 350mAh rechargeable LiPo battery. The battery management module is based on the TP4056 chip.
* **Enclosure:** The casing is fabricated using 3D printing.

## Repository Structure
* `ver1.4/`: The embedded program is developed in C++ based on the Arduino framework and is primarily responsible for sensor initialisation, continuous sampling, data filtering, differential computation, peak extraction, and BLE broadcasting.
* `modelTrain/`: Contains MATLAB scripts for BLE Collection and CSV Storage, and the Jupyter Notebook for SVM Model Training.
* `python_scripts/`: Contains Python scripts based on the pandas and scikit-learn frameworks for real-time inference, and the local web application built on the Flask framework for Feedback Generation.

## Dependencies
* **Arduino:** Requires the ArduinoBLE library and the SparkFun MAX3010x Sensor Library.
* **MATLAB:** The MATLAB BLE toolbox was employed for real-time data collection.
* **Python 3.x:** ```bash
  pip install pandas scikit-learn joblib bleak flask openai

## System Workflow
1. **Data Acquisition:** During this stage, the IMU continuously reads tri-axial motion data and transmits it to the computer, where it is stored as CSV files.
2. **Model Training:** Machine learning model training is conducted based on this dataset. The training process is performed in Jupyter Notebook, and a support vector machine (SVM) is employed as the classification model for this project.
3. **Inference:** A Python script receives the incoming data and applies the previously trained model to recognise which type of stroke the user has performed.
4. **Feedback Generation:** The Python script is also integrated with the ChatGPT API. The OpenAI ChatGPT API (gpt-40) was used to generate personalised coaching feedback. The front-end visualization is built based on the Flask framework.

## Author
**Yike Zhang**
School of Electronics and Computer Science
