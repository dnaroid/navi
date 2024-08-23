#include "BootDispatcher.h"
#include <secrets.h>
#include <vector>


void writeBootState(const BootState& state) {
  FILE* file = fopen(MODE_FILE, "w");
  if (!file) {
    LOG("Failed to open", MODE_FILE, "for writing");
    return;
  }
  fprintf(file, "%c,%.6f,%.6f,", state.mode, state.center.lon, state.center.lat);

  fprintf(file, "%d,", state.zoom);

  fprintf(file, "%.6f,%.6f,", state.target.lon, state.target.lat);

  fprintf(file, "%.6f,", state.distance);

  for (const auto& loc : state.route) {
    fprintf(file, "%.6f,%.6f,", loc.lon, loc.lat);
  }

  fclose(file);
  LOG("Boot state written to", MODE_FILE);
}


BootState readBootState() {
  BootState state = {ModeMap, {INIT_LON, INIT_LAT}, INIT_ZOOM, {0.0f, 0.0f}, {}, 0.0f};
  FILE* file = fopen(MODE_FILE, "r");
  if (!file) {
    LOG("Failed to open", MODE_FILE, "for reading");
    return state;
  }

  char modeChar;
  if (fscanf(file, "%c,", &modeChar) == 1) {
    state.mode = static_cast<Mode>(modeChar);
  }

  fscanf(file, "%f,%f,", &state.center.lon, &state.center.lat);

  fscanf(file, "%d,", &state.zoom);

  fscanf(file, "%f,%f,", &state.target.lon, &state.target.lat);

  fscanf(file, "%f,", &state.distance);

  float lon, lat;
  while (fscanf(file, "%f,%f,", &lon, &lat) == 2) {
    state.route.push_back({lon, lat});
  }

  fclose(file);
  LOG("Boot state read from", MODE_FILE);
  return state;
}
