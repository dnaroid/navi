#ifndef TOUCH_H
#define TOUCH_H

#include <misc/lv_area.h>

struct MultiTouchData {
  int direction;
  lv_point_t center;
  bool ready;
};

extern MultiTouchData mtZoom;
void Touch_init();


#endif //TOUCH_H
