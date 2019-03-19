#include <Arduino.h>
#include <U8g2lib.h>
#include "Projection.h"

U8G2_SH1107_64X128_F_4W_HW_SPI u8g2(U8G2_R0,14, /* dc=*/ 27, /* reset=*/ 33);

#define SCALE 1

float roll, pitch, yaw;
float scale = SCALE;

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
}

void loop() {
    updateCube();
    drawCube();
}

void updateCube() {
    // Update cube transform
    cubeTrans = Transform(roll += 2, pitch, yaw += 2, scale, scale, scale, 0, 0, 0);
}

void drawCube() {
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0, 10, "Hello World!");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display
  delay(1000);
}
