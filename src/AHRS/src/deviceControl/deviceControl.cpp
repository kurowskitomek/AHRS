#define _USE_MATH_DEFINES

#include <Arduino.h>
#include <math.h>
#include "deviceControl.hpp"
#include "../linAlg/Matrix.hpp"
#include "../linAlg/Vector.hpp"

float roundAngle(float angle)
{
    return std::roundf(angle * 10.0) / 10.0;
}

kf::DataPoint kalmanRun(
    kf::KalmanFilter &filter,
    kf::Measurement &measurement)
{
    kf::KalmanData &data = filter.getData();

    float mxh = measurement.mx * cos(data.x_hat(1)) + measurement.mz * sin(data.x_hat(1));
    float myh = measurement.mx * sin(data.x_hat(0)) * sin(data.x_hat(1)) + measurement.my * cos(data.x_hat(0)) - measurement.mz * sin(data.x_hat(0)) * cos(data.x_hat(1));

    float rollAcc = atan2(measurement.ay, measurement.az);
    float pitchAcc = atan2(-measurement.ax, sqrt(measurement.ay * measurement.ay + measurement.az * measurement.az));
    float yawMag = atan2(-myh, mxh);

    data.u = {measurement.gx, measurement.gy, measurement.gz};
    data.z = kf::Vector{rollAcc, pitchAcc, yawMag};

    filter.kalmanPredict();
    filter.kalmanUpdate();

    kf::DataPoint dataPoint = {
        measurement.millis,
        roundAngle(-data.x_hat(1) * RAD_TO_DEG),
        roundAngle(data.x_hat(0) * RAD_TO_DEG + 2.0f),
        roundAngle(-data.x_hat(2) * RAD_TO_DEG)};

    Serial.println("HDG: " + String(dataPoint.heading));

    return dataPoint;
}

void kalmanSetup(kf::KalmanData &data)
{
    data.I = kf::Identity(6);

    const float dt = 1.0 / FILTER_UPDATE_RATE_HZ;
    const float sigma_r = 0.15;
    const float sigma_p = 0.15;
    const float sigma_y = 3.0;

    const float qAngle = 0.001f;
    const float qBias = 0.0003f;

    data.F = {{1.0, 0.0, 0.0, -dt, 0.0, 0.0},
              {0.0, 1.0, 0.0, 0.0, -dt, 0.0},
              {0.0, 0.0, 1.0, 0.0, 0.0, -dt},
              {0.0, 0.0, 0.0, 1.0, 0.0, 0.0},
              {0.0, 0.0, 0.0, 0.0, 1.0, 0.0},
              {0.0, 0.0, 0.0, 0.0, 0.0, 1.0}};

    data.F_T = tp(data.F);

    data.G = {{dt, 0.0, 0.0},
              {0.0, dt, 0.0},
              {0.0, 0.0, dt},
              {0.0, 0.0, 0.0},
              {0.0, 0.0, 0.0},
              {0.0, 0.0, 0.0}};

    data.H = {{1.0, 0.0, 0.0, 0.0, 0.0, 0.0},
              {0.0, 1.0, 0.0, 0.0, 0.0, 0.0},
              {0.0, 0.0, 1.0, 0.0, 0.0, 0.0}};

    data.H_T = tp(data.H);

    data.R = {{sigma_r * sigma_r, 0.0, 0.0},
              {0.0, sigma_p * sigma_p, 0.0},
              {0.0, 0.0, sigma_y * sigma_y}};

    data.P = {{1, 0, 0, 0, 0, 0},
              {0, 1, 0, 0, 0, 0},
              {0, 0, 1, 0, 0, 0},
              {0, 0, 0, 10, 0, 0},
              {0, 0, 0, 0, 10, 0},
              {0, 0, 0, 0, 0, 10}};

    data.Q = {{qAngle, 0, 0, 0, 0, 0},
              {0, qAngle, 0, 0, 0, 0},
              {0, 0, qAngle, 0, 0, 0},
              {0, 0, 0, qBias, 0, 0},
              {0, 0, 0, 0, qBias, 0},
              {0, 0, 0, 0, 0, qBias}};

    data.x_hat = {0.0,
                  0.0,
                  0.0,
                  0.0,
                  0.0,
                  0.0};

    data.z = kf::Vector{0.0,
                        0.0,
                        0.0};

    data.u = {0.0,
              0.0,
              0.0};
}

DeviceControl::DeviceControl(
    Display &display,
    IMU &imu) : _display(display),
                _imu(imu)
{
}

void DeviceControl::powerOn(kf::KalmanData &kalmanData)
{
    Serial.begin(9600);
    Serial.println("Powering on...");

    _imu.initialize();
    _display.initializeLCD();
    // kalmanSetup(kalmanData);
    status = true;
}

void DeviceControl::shutDown()
{
    _display.turnOff();
    Serial.println("Shutting down...");
    Serial.end();
    status = false;
}

bool DeviceControl::getStatus() const
{
    return status;
}