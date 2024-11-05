#include "Filter.h"
#include <math.h>  

// Filter Coefficient
const float imuFilterAlpha = 0.1;

// Filerred Data
float ax_filtered = 0, ay_filtered = 0, az_filtered = 0;
float gx_filtered = 0, gy_filtered = 0, gz_filtered = 0;

// Normalised Data
float ax_normalized = 0, ay_normalized = 0, az_normalized = 0;
float gx_normalized = 0, gy_normalized = 0, gz_normalized = 0;
float roll_normalized = 0, pitch_normalized = 0;

void initFilter() {
  ax_filtered = ay_filtered = az_filtered = 0;
  gx_filtered = gy_filtered = gz_filtered = 0;
}

// LPF
void imuLowPassFilter(float ax, float ay, float az, float gx, float gy, float gz) {
  // 加速度计滤波
  ax_filtered = imuFilterAlpha * ax + (1 - imuFilterAlpha) * ax_filtered;
  ay_filtered = imuFilterAlpha * ay + (1 - imuFilterAlpha) * ay_filtered;
  az_filtered = imuFilterAlpha * az + (1 - imuFilterAlpha) * az_filtered;

  // 陀螺仪滤波
  gx_filtered = imuFilterAlpha * gx + (1 - imuFilterAlpha) * gx_filtered;
  gy_filtered = imuFilterAlpha * gy + (1 - imuFilterAlpha) * gy_filtered;
  gz_filtered = imuFilterAlpha * gz + (1 - imuFilterAlpha) * gz_filtered;
}

// Normalisation
void normalizeIMUData(float roll, float pitch) {
  //Datasheet defined
  ax_normalized = (ax_filtered + 4.0) / 8.0;
  ay_normalized = (ay_filtered + 4.0) / 8.0;
  az_normalized = (az_filtered + 4.0) / 8.0;

  gx_normalized = (gx_filtered + 2000.0) / 4000.0;
  gy_normalized = (gy_filtered + 2000.0) / 4000.0;
  gz_normalized = (gz_filtered + 2000.0) / 4000.0;

  roll_normalized = (roll + 180.0) / 360.0;
  pitch_normalized = (pitch + 180.0) / 360.0;
}

float calculateAverage(float *buffer, int size) {
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += buffer[i];
  }
  return sum / size;
}

bool isValidIMUData(float ax, float ay, float az, float gx, float gy, float gz) {
  if (abs(ax) > 16.0 || abs(ay) > 16.0 || abs(az) > 16.0) {
    return false;
  }
  if (abs(gx) > 2000.0 || abs(gy) > 2000.0 || abs(gz) > 2000.0) {
    return false;
  }
  return true;
}
