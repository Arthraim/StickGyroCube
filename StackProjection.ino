#include <Arduino.h>
#include <U8g2lib.h>
#include "Projection.h"
#include "MPU9250.h"
#include <string>

U8G2_SH1107_64X128_F_4W_HW_SPI u8g2(U8G2_R0,14, /* dc=*/ 27, /* reset=*/ 33);

#define UPDATE_DELAY  0
#define SCALE 1
#define STEP 0.5

float roll, pitch, yaw;

Transform cubeTrans;

// Cube vertices
point3 cubeVerts[] = {
    {  1,  1,  1 },
    {  1, -1,  1 },
    { -1, -1,  1 },
    { -1,  1,  1 },
    {  1,  1, -1 },
    {  1, -1, -1 },
    { -1, -1, -1 },
    { -1,  1, -1 }
};

// Cube lines
line3 cubeLines[] = {
    { cubeVerts[0], cubeVerts[1] },
    { cubeVerts[1], cubeVerts[2] },
    { cubeVerts[2], cubeVerts[3] },
    { cubeVerts[3], cubeVerts[0] },
    { cubeVerts[4], cubeVerts[5] },
    { cubeVerts[5], cubeVerts[6] },
    { cubeVerts[6], cubeVerts[7] },
    { cubeVerts[7], cubeVerts[4] },
    { cubeVerts[0], cubeVerts[4] },
    { cubeVerts[1], cubeVerts[5] },
    { cubeVerts[2], cubeVerts[6] },
    { cubeVerts[3], cubeVerts[7] },
};

// Create a camera
Camera cam(64, 128);

// MPU9250
MPU9250 IMU;

void setup() {
    u8g2.begin();

    // Camera starts out at origin, looking along world +Y axis (its own +Z axis).
    // Set camera back a few units so cube will be in view.
    cam.transform.y = -5;

    // Setup MPU9250
    Wire.begin(21, 22, 100000);
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
    IMU.initMPU9250();
    IMU.initAK8963(IMU.magCalibration);
}

void loop() {
    u8g2.clearBuffer();

    updateCube();
    drawCube();

    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 7, "@ArthurWXY");
    showGyroState();

    u8g2.sendBuffer();
    delay(UPDATE_DELAY);
}

void updateCube() {
    // Update cube transform
    cubeTrans = Transform(roll += STEP, pitch, yaw += STEP, SCALE, SCALE, SCALE, 0, 0, 0);
}

void drawCube() {
    for (byte i = 0; i < 12; i++) {
        // Project line to screen after applying cube transform
        line2 line = cam.project(cubeTrans * cubeLines[i]);

        // Draw if not clipped completely
        if (!isnan(line.p0.x)) {
            u8g2.drawLine(line.p0.x, line.p0.y, line.p1.x, line.p1.y);
        }
    }
}

void showGyroState() {
    // If intPin goes high, all data registers have new data
    // On interrupt, check if data ready interrupt
    if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01) {
        IMU.readAccelData(IMU.accelCount);
        IMU.getAres();

        IMU.ax = (float)IMU.accelCount[0] * IMU.aRes - IMU.accelBias[0];
        IMU.ay = (float)IMU.accelCount[1] * IMU.aRes - IMU.accelBias[1];
        IMU.az = (float)IMU.accelCount[2] * IMU.aRes - IMU.accelBias[2];

        IMU.readGyroData(IMU.gyroCount);  // Read the x/y/z adc values
        IMU.getGres();

        // Calculate the gyro value into actual degrees per second
        // This depends on scale being set
        IMU.gx = (float)IMU.gyroCount[0] * IMU.gRes;
        IMU.gy = (float)IMU.gyroCount[1] * IMU.gRes;
        IMU.gz = (float)IMU.gyroCount[2] * IMU.gRes;

        IMU.readMagData(IMU.magCount);  // Read the x/y/z adc values
        IMU.getMres();
        // User environmental x-axis correction in milliGauss, should be
        // automatically calculated
        //IMU.magbias[0] = +470.;
        // User environmental x-axis correction in milliGauss TODO axis??
        //IMU.magbias[1] = +120.;
        // User environmental x-axis correction in milliGauss
        //IMU.magbias[2] = +125.;

        // Calculate the magnetometer values in milliGauss
        // Include factory calibration per data sheet and user environmental
        // corrections
        // Get actual magnetometer value, this depends on scale being set
        IMU.mx = (float)IMU.magCount[0] * IMU.mRes * IMU.magCalibration[0] -
                IMU.magbias[0];
        IMU.my = (float)IMU.magCount[1] * IMU.mRes * IMU.magCalibration[1] -
                IMU.magbias[1];
        IMU.mz = (float)IMU.magCount[2] * IMU.mRes * IMU.magCalibration[2] -
                IMU.magbias[2];

        IMU.tempCount = IMU.readTempData();  // Read the adc values
        // Temperature in degrees Centigrade
        IMU.temperature = ((float) IMU.tempCount) / 333.87 + 21.0;

        // Accelerometer in mg
        int ax = (int)(1000 * IMU.ax);
        int ay = (int)(1000 * IMU.ay);
        int az = (int)(1000 * IMU.az);

        // Gyroscope in o/s
        int gx = (int)(IMU.gx);
        int gy = (int)(IMU.gy);
        int gz = (int)(IMU.gz);

        // Magnetometer in mG
        int mx = (int)(IMU.mx);
        int my = (int)(IMU.my);
        int mz = (int)(IMU.mz);

        drawXYZ(ax, ay, az);
        // drawXYZ(gx, gy, gz);
        // drawXYZ(mx, my, mz);
    }
}

void drawXYZ(int x, int y, int z) {
    char buffer[20];
    itoa(x, buffer,10);
    u8g2.drawStr(0, 128-2*7, buffer);
    itoa(y, buffer,10);
    u8g2.drawStr(0, 128-1*7, buffer);
    itoa(z, buffer,10);
    u8g2.drawStr(0, 128-0*7, buffer);
}
