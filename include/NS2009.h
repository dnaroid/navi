#ifndef NS2009_H
#define NS2009_H

#define NS2009_ADDR 0x48  // 10010000

#define NS2009_LOW_POWER_READ_X 0xc0
#define NS2009_LOW_POWER_READ_Y 0xd0
#define NS2009_LOW_POWER_READ_Z1 0xe0
#define NS2009_LOW_POWER_READ_Z2 0xf0

bool initTouch();
bool ns2009_pos(int pos[2]);

#endif // NS2009_H
