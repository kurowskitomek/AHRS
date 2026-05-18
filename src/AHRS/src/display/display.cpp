#include "display.hpp"
#include "Arduino.h"

void Display::LCDPrintTest()
{
    lcd.setTextWrap(false);
    lcd.fillScreen(ST77XX_BLACK);
    lcd.setCursor(40, 160);
    lcd.setTextColor(ST77XX_WHITE);
    lcd.setTextSize(2);
    lcd.println("INITIALIZE AHRS");
    delay(1000);
    lcd.fillScreen(ST77XX_BLACK);
    lcd.setCursor(20, 64);
    lcd.setTextColor(ST77XX_WHITE);
    lcd.setTextSize(2);

    delay(100);

    lcd.setTextColor(ST77XX_GREEN);
    lcd.fillScreen(ST77XX_BLACK);
    lcd.setTextSize(3);
    lcd.setCursor(80, 160);
    lcd.println("READY");

    delay(1000);
}

void drawAircraft(GFXcanvas16 &horizonCanvas)
{
    float arm = 28.0f;
    float drop = 14.0f;

    float outerHalf = 10.0f; // wing tip thickness
    float innerHalf = 3.0f;  // center tip thickness

    // helper vectors (arms are straight in screen space)
    float leftX1 = CENTER_X - arm;
    float leftY1 = CENTER_Y + drop;

    float rightX1 = CENTER_X + arm;
    float rightY1 = CENTER_Y + drop;

    // LEFT ARM (proper trapezoid)
    horizonCanvas.fillTriangle(
        leftX1 - outerHalf, leftY1,
        leftX1 + outerHalf, leftY1,
        CENTER_X - innerHalf, CENTER_Y,
        ST77XX_YELLOW);

    horizonCanvas.fillTriangle(
        leftX1 + outerHalf, leftY1,
        CENTER_X + innerHalf, CENTER_Y,
        CENTER_X - innerHalf, CENTER_Y,
        ST77XX_YELLOW);

    horizonCanvas.fillTriangle(
        leftX1 + outerHalf, leftY1,
        CENTER_X + innerHalf, CENTER_Y,
        CENTER_X - innerHalf, CENTER_Y,
        ST77XX_YELLOW);

    // RIGHT ARM
    horizonCanvas.fillTriangle(
        rightX1 - outerHalf, rightY1,
        rightX1 + outerHalf, rightY1,
        CENTER_X - innerHalf, CENTER_Y,
        ST77XX_YELLOW);

    horizonCanvas.fillTriangle(
        rightX1 + outerHalf, rightY1,
        CENTER_X + innerHalf, CENTER_Y,
        CENTER_X - innerHalf, CENTER_Y,
        ST77XX_YELLOW);
}

void drawPitchLadder(GFXcanvas16 &horizonCanvas, float pitch, float roll, float px, float py, float dx, float dy)
{
    for (float pitchMark = -30.0f; pitchMark <= 30.0f; pitchMark += 2.5f)
    {
        // Skip center horizon line
        if (fabs(pitchMark) < 0.01f)
            continue;

        // Relative pitch position
        float offset = (pitch - pitchMark) * PIXELS_PER_DEGREE;

        // Center of this pitch line
        float lx = CENTER_X + px * offset;
        float ly = CENTER_Y + py * offset;

        // Line width
        int halfWidth;

        // Longer line every 10 deg
        if (fmod(fabs(pitchMark), 10.0f) < 0.1f)
            halfWidth = 40;
        else if (fmod(fabs(pitchMark), 5.0f) < 0.1f)
            halfWidth = 28;
        else
            halfWidth = 18;

        // Pitch line endpoints
        int px1 = lx - dx * halfWidth;
        int py1 = ly - dy * halfWidth;

        int px2 = lx + dx * halfWidth;
        int py2 = ly + dy * halfWidth;

        // Draw pitch line
        horizonCanvas.drawLine(px1, py1, px2, py2, ST77XX_WHITE);

        // Optional labels every 10 deg
        if (fmod(fabs(pitchMark), 10.0f) < 0.1f)
        {
            horizonCanvas.setTextColor(ST77XX_WHITE);
            horizonCanvas.setTextSize(1);

            // LEFT LABEL
            if (px1 > 15 && px1 < HORIZON_W - 25 && py1 > 5 && py1 < HORIZON_H - 10)
            {
                horizonCanvas.setCursor(px1 - 15, py1 - 3);
                horizonCanvas.print((int)fabs(pitchMark));
            }

            // RIGHT LABEL
            if (px2 > 5 && px2 < HORIZON_W - 20 && py2 > 5 && py2 < HORIZON_H - 10)
            {
                horizonCanvas.setCursor(px2 + 5, py2 - 3);
                horizonCanvas.print((int)fabs(pitchMark));
            }
        }
    }
}

void drawBankScale(GFXcanvas16 &horizonCanvas, float roll)
{
    // Rotating bank scale
    const int bankRadius = 100;

    for (int angle = -60; angle <= 60; angle += 10)
    {
        // Total rotated angle
        float a = (angle + (roll / DEG_TO_RAD) - 90.0f) * DEG_TO_RAD;

        // Tick size
        int innerRadius;

        if (angle % 30 == 0)
            innerRadius = bankRadius - 12;
        else
            innerRadius = bankRadius - 6;

        // Outer point
        int x1 = CENTER_X + cos(a) * bankRadius;
        int y1 = CENTER_Y + sin(a) * bankRadius;

        // Inner point
        int x2 = CENTER_X + cos(a) * innerRadius;
        int y2 = CENTER_Y + sin(a) * innerRadius;

        horizonCanvas.drawLine(x1, y1, x2, y2, ST77XX_WHITE);
    }

    // Rotating zero-bank triangle

    // Angle at current roll
    float a = ((roll / DEG_TO_RAD) - 90.0f) * DEG_TO_RAD;

    // Triangle geometry
    int outerR = bankRadius;
    int baseR = bankRadius + 12;

    // Triangle tip
    int tx = CENTER_X + cos(a) * outerR;
    int ty = CENTER_Y + sin(a) * outerR;

    // Perpendicular vector
    float nx = -sin(a);
    float ny = cos(a);

    // Base center
    int bx = CENTER_X + cos(a) * baseR;
    int by = CENTER_Y + sin(a) * baseR;

    // Triangle base corners
    int lx = bx + nx * 6;
    int ly = by + ny * 6;

    int rx = bx - nx * 6;
    int ry = by - ny * 6;

    // Draw triangle
    horizonCanvas.fillTriangle(tx, ty, lx, ly, rx, ry, ST77XX_WHITE);

    // Fixed roll pointer
    horizonCanvas.fillTriangle(CENTER_X - 7, 44, CENTER_X + 7, 44, CENTER_X, 32, ST77XX_WHITE);
}

void drawHorizon(GFXcanvas16 &horizonCanvas, float pitch, float roll, float px, float py, float dx, float dy)
{
    // Pitch scaling
    float pitchOffset = pitch * PIXELS_PER_DEGREE;

    // Shifted center
    float cx = CENTER_X + px * pitchOffset;
    float cy = CENTER_Y + py * pitchOffset;

    // Long horizon line
    const int lineLength = 400;

    int x1 = cx - dx * lineLength;
    int y1 = cy - dy * lineLength;

    int x2 = cx + dx * lineLength;
    int y2 = cy + dy * lineLength;

    // Draw sky background
    horizonCanvas.fillRect(HORIZON_X, HORIZON_Y, HORIZON_W, HORIZON_H, AHRS_BLUE);

    // Ground quad
    float gx1 = x1 + px * 1000;
    float gy1 = y1 + py * 1000;

    float gx2 = x2 + px * 1000;
    float gy2 = y2 + py * 1000;

    // Ground polygon (2 triangles)
    horizonCanvas.fillTriangle(x1, y1, x2, y2, gx1, gy1, AHRS_BROWN);
    horizonCanvas.fillTriangle(gx1, gy1, gx2, gy2, x2, y2, AHRS_BROWN);

    // Horizon line
    horizonCanvas.drawLine(x1, y1, x2, y2, ST77XX_WHITE);
}

void Display::showAttitude(float pitch, float roll)
{
    horizonCanvas.fillRect(HORIZON_X, HORIZON_Y, HORIZON_W, HORIZON_H, ST77XX_BLACK);

    roll *= DEG_TO_RAD;

    // Horizon direction vector
    float dx = cos(roll);
    float dy = sin(roll);

    // Perpendicular vector for pitch
    float px = -dy;
    float py = dx;

    drawHorizon(horizonCanvas, pitch, roll, px, py, dx, dy);
    drawPitchLadder(horizonCanvas, pitch, roll, px, py, dx, dy);
    drawBankScale(horizonCanvas, roll);
    drawAircraft(horizonCanvas);

    lcd.drawRGBBitmap(0, 0, horizonCanvas.getBuffer(), horizonCanvas.width(), horizonCanvas.height());
}

void Display::showCompass(float heading)
{
    compassCanvas.fillScreen(ST77XX_BLACK);

    const int CX = COMPASS_W / 2;
    const int CY = COMPASS_H + 20;

    const int R = 90;

    // Draw arc ticks
    for (int deg = 0; deg < 360; deg += 5)
    {
        // Relative angle
        float relative = (deg - heading) * DEG_TO_RAD;

        float a = relative - HALF_PI;

        // Only visible upper semicircle
        if (cos(relative) < 0)
            continue;

        // Tick lengths
        int innerR;

        if (deg % 30 == 0)
            innerR = R - 15;
        else if (deg % 10 == 0)
            innerR = R - 10;
        else
            innerR = R - 5;

        // Tick endpoints
        int x1 = CX + cos(a) * R;
        int y1 = CY + sin(a) * R;

        int x2 = CX + cos(a) * innerR;
        int y2 = CY + sin(a) * innerR;

        compassCanvas.drawLine(x1, y1, x2, y2, ST77XX_WHITE);

        // Labels every 30 deg
        if (deg % 30 == 0)
        {
            int textR = R - 28;

            int tx = CX + cos(a) * textR;
            int ty = CY + sin(a) * textR;

            String label;

            switch (deg)
            {
            case 0:
                label = "N";
                break;
            case 90:
                label = "E";
                break;
            case 180:
                label = "S";
                break;
            case 270:
                label = "W";
                break;

            default:
                label = String(deg / 10);
                break;
            }

            compassCanvas.setTextColor(ST77XX_WHITE);
            compassCanvas.setTextSize(1);

            int16_t x, y;
            uint16_t w, h;

            compassCanvas.getTextBounds(label, 0, 0, &x, &y, &w, &h);
            compassCanvas.setCursor(tx - w / 2, ty - h / 2);
            compassCanvas.print(label);
        }
    }

    // Fixed aircraft heading marker
    compassCanvas.fillTriangle(CX - 6, 1, CX + 6, 1, CX, 12, ST77XX_WHITE);

    lcd.drawRGBBitmap(0, 240, compassCanvas.getBuffer(), compassCanvas.width(), compassCanvas.height());
}

void Display::initializeLCD()
{
    digitalWrite(LCD_CS, LOW);
    lcd.init(SCREEN_W, SCREEN_H, SPI_MODE3);
    lcd.setSPISpeed(40000000);
    lcd.enableDisplay(1);
    lcd.setRotation(2);
    LCDPrintTest();
}

void Display::turnOff()
{
    lcd.fillScreen(ST77XX_BLACK);
    digitalWrite(LCD_BCKLIT, LOW);
    lcd.enableDisplay(0);
}