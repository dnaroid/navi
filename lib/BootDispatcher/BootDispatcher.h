#ifndef REBOOTDISPATCHER_H
#define REBOOTDISPATCHER_H

#define MODE_FILE "/sd/mode.txt"

#include <globals.h>
#include <vector>

enum Mode {
  ModeMap = 'm',
  ModeRoute = 'r',
  ModeMirror = 'b'
};

struct BootState {
  Mode mode;
  Location center;
  int zoom;
  Location start;
  Location end;
  std::vector<Location> route;
  float distance;
};

void writeBootState(const BootState& state);
BootState readBootState();

#endif //REBOOTDISPATCHER_H
