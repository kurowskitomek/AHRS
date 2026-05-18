#ifndef _DISPLAY_
#define _DISPLAY_

#include <Adafruit_ST7789.h>
#include "Adafruit_GFX.h"

#define ATT_REFRESH_RATE_HZ 40
#define HDG_THRESHOLD_DEG 1.0
#define ATT_THRESHOLD_DEG 0.2

#define LCD_CS 10
#define LCD_DC 7
#define LCD_RST -1
#define TP_CS 4
#define TP_IRQ 3
#define LCD_BCKLIT 9

#define PIXELS_PER_DEGREE 7.0f

#define HORIZON_X 0
#define HORIZON_Y 0

#define HORIZON_W 240
#define HORIZON_H 240

#define COMPASS_X = 0
#define COMPASS_Y = 240

#define COMPASS_W 240
#define COMPASS_H 80

#define SCREEN_W 240
#define SCREEN_H 320

#define CENTER_X (HORIZON_W / 2)
#define CENTER_Y (HORIZON_H / 2)

#define AHRS_BLUE 0x04ff
#define AHRS_BROWN 0xb400

class Display
{
private:
    GFXcanvas16 horizonCanvas = GFXcanvas16(HORIZON_W, HORIZON_H);
    GFXcanvas16 compassCanvas = GFXcanvas16(SCREEN_W, SCREEN_H);
    Adafruit_ST7789 lcd = Adafruit_ST7789(LCD_CS, LCD_DC, LCD_RST);

public:
    void turnOff();
    void LCDPrintTest();
    void initializeLCD();
    void showAttitude(float pitch, float roll);
    void showCompass(float heading);
};

#endif