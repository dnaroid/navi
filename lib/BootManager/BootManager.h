#ifndef BOOTMANAGER_H
#define BOOTMANAGER_H

#define MODE_FILE "/sd/boot.cfg"

#include <globals.h>
#include <vector>

#define CURRENT_BM_VER 0

enum Mode {
  ModeMap = 'm',
  ModeRoute = 'r',
  ModeDrive = 'd',
};

enum Transport {
  TransportAll = 'a',
  TransportBike = 'b',
  TransportCar = 'c',
  TransportWalk = 'w',
};

struct BootState {
  char version;
  Mode mode;
  Transport transport;
  Location center;
  int zoom;
  Location start;
  Location end;
  float distance;
  std::vector<Location> route;
};

void writeBootState(const BootState& state);
void switchBootMode(Mode mode);
BootState readBootState();

#endif //BOOTMANAGER_H
