#include <Adafruit_AHRS_FusionInterface.h>
#include <Adafruit_AHRS_NXPFusion.h>
#include <Adafruit_AHRS.h>
#include <Adafruit_Sensor_Set.h>
#include <Adafruit_Simple_AHRS.h>

#include <Arduino.h>

#include "./src/deviceControl/deviceControl.hpp"
#include "./src/display/display.hpp"
#include "./src/kalmanFilter/KalmanFilter.hpp"
#include "./src/linAlg/Matrix.hpp"
#include "./src/linAlg/Vector.hpp"
#include "./src/display/display.hpp"
#include "./src/sensor/imu.hpp"

Display display;
IMU imu;
kf::KalmanData kalmanData;
kf::DataPoint currentDataPoint;
DeviceControl deviceControl(display, imu);
kf::KalmanFilter kalmanFilter(kalmanData);

kf::DataPoint dataPoint;
SemaphoreHandle_t attitudeMutex;

void taskAHRS(void *pvParameters);
void taskDisplay(void *pvParameters);

void setup()
{
    pinMode(LCD_CS, OUTPUT);
    digitalWrite(LCD_CS, HIGH);
    pinMode(TP_CS, OUTPUT);
    digitalWrite(TP_CS, HIGH);
    pinMode(LCD_BCKLIT, OUTPUT);
    digitalWrite(LCD_BCKLIT, HIGH);

    deviceControl.powerOn(kalmanData);

    attitudeMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(taskAHRS, "AHRS", 8192, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(taskDisplay, "DISPLAY", 16384, NULL, 1, NULL, 0);
}

void taskAHRS(void *pvParameters)
{
    // kf::Measurement measurement = {0, 0.0, 0.0, 0.0};
    kf::DataPoint localDataPoint = {0, 0.0, 0.0, 0.0};

    while (true)
    {
        if (imu.read(localDataPoint) != 0)
            continue;

        // localDataPoint = kalmanRun(kalmanFilter, measurement);

        if (xSemaphoreTake(attitudeMutex, portMAX_DELAY))
        {
            dataPoint.pitch = localDataPoint.pitch;
            dataPoint.roll = localDataPoint.roll;
            dataPoint.heading = localDataPoint.heading;

            xSemaphoreGive(attitudeMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(1000.0f / FILTER_UPDATE_RATE_HZ));
    }
}

void taskDisplay(void *pvParameters)
{
    float pitch;
    float roll;
    float heading;

    bool updateAtt = false;
    bool updateHdg = false;

    while (true)
    {
        if (xSemaphoreTake(attitudeMutex, portMAX_DELAY))
        {
            if (fabs(dataPoint.pitch - pitch) > ATT_THRESHOLD_DEG)
            {
                pitch = dataPoint.pitch;
                updateAtt = true;
            }

            if (fabs(dataPoint.roll - roll) > ATT_THRESHOLD_DEG)
            {
                roll = dataPoint.roll;
                updateAtt = true;
            }

            if (fabs(dataPoint.heading - heading) > HDG_THRESHOLD_DEG)
            {
                heading = dataPoint.heading;
                updateHdg = true;
            }

            xSemaphoreGive(attitudeMutex);
        }

        if (updateAtt)
        {
            display.showAttitude(pitch, roll);
            updateAtt = false;
        }

        if (updateHdg)
        {
            display.showCompass(heading);
            updateHdg = false;
        }

        vTaskDelay(pdMS_TO_TICKS(1000.0f / ATT_REFRESH_RATE_HZ));
    }
}

void loop()
{
}