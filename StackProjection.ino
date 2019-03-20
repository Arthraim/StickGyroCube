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
int ax, ay, az;
int deltaX, deltaY, deltaZ;

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
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 7, "@ArthurWXY");

    updateAccelerometerState();
    updateCube();
    drawCube();

    u8g2.sendBuffer();
    delay(UPDATE_DELAY);
}

void updateCube() {
    // Update cube transform
    cubeTrans = Transform(
        roll -= (deltaY / 100), // += (deltaX / 100),
        pitch, // += (deltaY / 100),
        yaw += (deltaX / 100), // += (deltaZ / 100),
        SCALE, SCALE, SCALE,
        0, 0, 0
        );
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

void updateAccelerometerState() {
    // If intPin goes high, all data registers have new data
    // On interrupt, check if data ready interrupt
    if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01) {
        IMU.readAccelData(IMU.accelCount);
        IMU.getAres();

        IMU.ax = (float)IMU.accelCount[0] * IMU.aRes - IMU.accelBias[0];
        IMU.ay = (float)IMU.accelCount[1] * IMU.aRes - IMU.accelBias[1];
        IMU.az = (float)IMU.accelCount[2] * IMU.aRes - IMU.accelBias[2];

        // Accelerometer in mg
        if (ax == 0 || ay == 0 || az == 0) {
            ax = (int)(1000 * IMU.ax);
            ay = (int)(1000 * IMU.ay);
            az = (int)(1000 * IMU.az);
        } else {
            deltaX = (int)(1000 * IMU.ax) - ax;
            deltaY = (int)(1000 * IMU.ay) - ay;
            deltaZ = (int)(1000 * IMU.az) - az;
        }

        drawXYZ(deltaX, deltaY, deltaZ);

        char buffer[20];
        itoa(ax, buffer,10);
        u8g2.drawStr(35, 128-2*7, buffer);
        itoa(ay, buffer,10);
        u8g2.drawStr(35, 128-1*7, buffer);
        itoa(az, buffer,10);
        u8g2.drawStr(35, 128-0*7, buffer);
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
