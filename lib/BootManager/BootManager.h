#ifndef BOOTMANAGER_H
#define BOOTMANAGER_H

#define MODE_FILE "/sd/boot.cfg"

#include <globals.h>
#include <vector>

#define CURRENT_BM_VER 0

enum Mode {
  ModeMap = 'm',
  ModeRoute = 'r',
  ModeMirror = 'b',
};

struct BootState {
  char version;
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

#endif //BOOTMANAGER_H
