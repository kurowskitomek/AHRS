#include "imu.hpp"
#include <Arduino.h>

void IMU::initialize()
{
    // delay(10000);

    if (!cal.begin())
    {
        Serial.println("Failed to initialize calibration helper");
    }
    else if (!cal.loadCalibration())
    {
        Serial.println("No calibration loaded/found");
    }

    // delay(10000);

    Serial.println("Adafruit LSM6DS+LIS3MDL initialization");

    bool lsm6ds_success = lsm6ds.begin_I2C();
    bool lis3mdl_success = lis3mdl.begin_I2C();

    if (!lsm6ds_success)
        Serial.println("Failed to find LSM6DS chip");

    if (!lis3mdl_success)
        Serial.println("Failed to find LIS3MDL chip");

    if (!(lsm6ds_success && lis3mdl_success))
        while (1)
            delay(10);

    accelerometer = lsm6ds.getAccelerometerSensor();
    gyroscope = lsm6ds.getGyroSensor();
    magnetometer = &lis3mdl;
    temperature = lsm6ds.getTemperatureSensor();

    accelerometer->printSensorDetails();
    gyroscope->printSensorDetails();
    magnetometer->printSensorDetails();
    temperature->printSensorDetails();

    // set lowest range
    lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
    lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
    lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);

    // set slightly above refresh rate
    lsm6ds.setAccelDataRate(LSM6DS_RATE_104_HZ);
    lsm6ds.setGyroDataRate(LSM6DS_RATE_104_HZ);
    lis3mdl.setDataRate(LIS3MDL_DATARATE_1000_HZ);
    lis3mdl.setPerformanceMode(LIS3MDL_MEDIUMMODE);
    lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE);

    filter.begin(FILTER_UPDATE_RATE_HZ);
    // timestamp = millis();

    Wire.setClock(400000);

    Serial.println("LSM6DS and LIS3MDL initialized successfully");

    /*lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
    lsm6ds.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
    lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
    lsm6ds.setGyroDataRate(LSM6DS_RATE_12_5_HZ);
    lis3mdl.setDataRate(LIS3MDL_DATARATE_155_HZ);
    lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);
    lis3mdl.setPerformanceMode(LIS3MDL_MEDIUMMODE);
    lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE);
    lis3mdl.setIntThreshold(500);
    lis3mdl.configInterrupt(false, false, true, // enable z axis
                            true,               // polarity
                            false,              // don't latch
                            true);              // enabled!*/
}

int IMU::read(kf::DataPoint &dataPoint)
{
    sensors_event_t accel, gyro, mag, temp;

    accelerometer->getEvent(&accel);
    gyroscope->getEvent(&gyro);
    magnetometer->getEvent(&mag);
    temperature->getEvent(&temp);

    cal.calibrate(mag);
    cal.calibrate(accel);
    cal.calibrate(gyro);

    kf::Measurement measurement = kf::Measurement{
        .millis = millis(),
        .ax = -accel.acceleration.y,
        .ay = accel.acceleration.x,
        .az = accel.acceleration.z,
        .gx = -gyro.gyro.y * SENSORS_RADS_TO_DPS,
        .gy = gyro.gyro.x * SENSORS_RADS_TO_DPS,
        .gz = gyro.gyro.z * SENSORS_RADS_TO_DPS,
        .mx = -mag.magnetic.y,
        .my = mag.magnetic.x,
        .mz = mag.magnetic.z,
        .temp = temp.temperature};

    filter.update(measurement.gx, measurement.gy, measurement.gz,
                  measurement.ax, measurement.ay, measurement.az,
                  measurement.mx, measurement.my, measurement.mz);

    dataPoint = kf::DataPoint{
        .millis = measurement.millis,
        .roll = filter.getRoll(),
        .pitch = filter.getPitch(),
        .heading = fmod(-filter.getYaw() - 90.0f, 360.0f)};

    return 0;
}