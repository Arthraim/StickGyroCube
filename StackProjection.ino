#include <Arduino.h>
#include <U8g2lib.h>
#include "Projection.h"

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

void setup() {
    u8g2.begin();

    // Camera starts out at origin, looking along world +Y axis (its own +Z axis).
    // Set camera back a few units so cube will be in view.
    cam.transform.y = -5;}

void loop() {
    u8g2.clearBuffer();

    updateCube();
    drawCube();

    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 7, " @ArthurWXY");

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
        if (!isnan(line.p0.x))
        {
            u8g2.drawLine(line.p0.x, line.p0.y, line.p1.x, line.p1.y);
        }
    }
}
