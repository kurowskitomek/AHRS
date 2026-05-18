#ifndef __DEVICE_CONTROL__
#define __DEVICE_CONTROL__

#include "../display/display.hpp"
#include "../sensor/imu.hpp"

void kalmanSetup(kf::KalmanData &data);

kf::DataPoint kalmanRun(
    kf::KalmanFilter &filter,
    kf::Measurement &measurement);

class DeviceControl
{
private:
    Display &_display;
    IMU &_imu;
    bool status = false;

public:
    DeviceControl(Display &display, IMU &imu);

    void powerOn(kf::KalmanData &kalmanData);
    void shutDown();
    bool getStatus() const;
};

#endif