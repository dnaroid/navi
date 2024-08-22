#include "NS2009.h"

#include <cstdint>
#include <Wire.h>

#include "globals.h"


// calibration data
static int rawTouchXMin = 380;
static int rawTouchXMax = 3835;
static int rawTouchYMin = 333;
static int rawTouchYMax = 3903;

static int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool initTouch() {
  byte error, address;
  Wire.beginTransmission(NS2009_ADDR);
  error = Wire.endTransmission();
  return error == 0;
}

// I2C receive
static void ns2009_recv(const uint8_t* send_buf, size_t send_buf_len, uint8_t* receive_buf, size_t receive_buf_len) {
  Wire.beginTransmission(NS2009_ADDR);
  Wire.write(send_buf, send_buf_len);
  Wire.endTransmission();
  Wire.requestFrom(NS2009_ADDR, receive_buf_len);
  while (Wire.available()) {
    *receive_buf++ = Wire.read();
  }
}

// read 12bit data
static unsigned int ns2009_read(uint8_t cmd) {
  uint8_t buf[2];
  ns2009_recv(&cmd, 1, buf, 2);
  return (buf[0] << 4) | (buf[1] >> 4);
}

bool ns2009_pos(int pos[2]) {
  unsigned int z = ns2009_read(NS2009_LOW_POWER_READ_Z1);
  if (z < 30) { return false; }

  unsigned int x = ns2009_read(NS2009_LOW_POWER_READ_X);
  unsigned int y = ns2009_read(NS2009_LOW_POWER_READ_Y);
  unsigned int z2 = ns2009_read(NS2009_LOW_POWER_READ_Z2);

  if (y >= 4095 || x >= 4095 || z2 >= 4095) return false;

  int sx = map(x, rawTouchXMin, rawTouchXMax, SCREEN_WIDTH, 0);
  int sy = map(y, rawTouchYMin, rawTouchYMax, SCREEN_HEIGHT, 0);

  // print("-> NS2009.cpp:57 sx,sy,z:", sx, sy, z);
  //if (sx < 0 || sx > SCREEN_WIDTH || sy < 0 || sy > SCREEN_HEIGHT) { return false; }
  pos[0] = sx;
  pos[1] = sy;
  return true;
}
