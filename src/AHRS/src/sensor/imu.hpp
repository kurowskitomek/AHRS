#ifndef __IMU__
#define __IMU__

#include <Adafruit_LIS3MDL.h>
#include <Adafruit_LSM6DS3TRC.h>
#include "../kalmanFilter/KalmanFilter.hpp"
#include <Adafruit_Sensor_Calibration_EEPROM.h>
#include <Adafruit_Sensor_Calibration.h>
#include <Adafruit_AHRS.h>

#define FILTER_UPDATE_RATE_HZ 30

class IMU
{
private:
    unsigned long timestamp = 0;
    float ax, ay, az;
    float gx, gy, gz;

    float yawTotal = 0.0;

    Adafruit_LSM6DS3TRC lsm6ds; //= Adafruit_LSM6DS3TRC();
    Adafruit_LIS3MDL lis3mdl;   //= Adafruit_LIS3MDL();
    Adafruit_Sensor_Calibration_EEPROM cal;
    Adafruit_Sensor *accelerometer, *gyroscope, *magnetometer, *temperature;
    Adafruit_NXPSensorFusion filter;

public:
    void initialize();
    int read(kf::DataPoint &dataPoint);
};

#endif