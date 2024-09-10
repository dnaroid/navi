#include <globals.h>
#include <Wire.h>
#include <LSM303.h>
#include <fstream>
#include <iostream>

struct CompassCalibrationData {
  LSM303::vector<int16_t> m_min;
  LSM303::vector<int16_t> m_max;
};

CompassCalibrationData cal_data;
const char* settingsFile = "/sd/compass.cfg";
LSM303::vector<int16_t> running_min = {32767, 32767, 32767}, running_max = {-32768, -32768, -32768};
char report[80];

bool loadCalibrationData(LSM303& compass) {
  std::ifstream file(settingsFile);
  if (file.is_open()) {
    file >> cal_data.m_min.x >> cal_data.m_min.y >> cal_data.m_min.z;
    file >> cal_data.m_max.x >> cal_data.m_max.y >> cal_data.m_max.z;
    file.close();
    compass.m_min = cal_data.m_min;
    compass.m_max = cal_data.m_max;
    LOG("Calibration data loaded successfully");
    return true;
  }
  LOG("Could not open settings file");
  return false;
}

void saveCalibrationData() {
  std::ofstream file(settingsFile);
  if (file.is_open()) {
    file << cal_data.m_min.x << " " << cal_data.m_min.y << " " << cal_data.m_min.z << std::endl;
    file << cal_data.m_max.x << " " << cal_data.m_max.y << " " << cal_data.m_max.z << std::endl;
    file.close();
    LOG("Calibration data saved successfully");
  }
}

void calibrateCompass(LSM303& compass) {
  for (int i = 0; i < 1000; i++) {
    compass.read();

    running_min.x = min(running_min.x, compass.m.x);
    running_min.y = min(running_min.y, compass.m.y);
    running_min.z = min(running_min.z, compass.m.z);

    running_max.x = max(running_max.x, compass.m.x);
    running_max.y = max(running_max.y, compass.m.y);
    running_max.z = max(running_max.z, compass.m.z);

    snprintf(report, sizeof(report), "min: {%+6d, %+6d, %+6d}    max: {%+6d, %+6d, %+6d}",
             running_min.x, running_min.y, running_min.z,
             running_max.x, running_max.y, running_max.z);
    Serial.println(report);
    delay(10);
  }

  cal_data.m_min.x = running_min.x;
  cal_data.m_min.y = running_min.y;
  cal_data.m_min.z = running_min.z;
  cal_data.m_max.x = running_max.x;
  cal_data.m_max.y = running_max.y;
  cal_data.m_max.z = running_max.z;

  saveCalibrationData();

  compass.m_min = cal_data.m_min;
  compass.m_max = cal_data.m_max;
}
