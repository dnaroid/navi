#include "BootManager.h"
#include <iostream>
#include <vector>


const char* modeToString(const Mode mode) {
  switch (mode) {
  case ModeMap: return "Map";
  case ModeRoute: return "Route";
  case ModeMirror: return "Mirror";
  default: return "Unknown";
  }
}

void printBootState(const BootState& state) {
  std::cout << "BootState:" << std::endl;
  std::cout << "  Mode: " << modeToString(state.mode) << std::endl;
  std::cout << "  Center: (" << state.center.lon << ", " << state.center.lat << ")" << std::endl;
  std::cout << "  Zoom: " << state.zoom << std::endl;
  std::cout << "  Start: (" << state.start.lon << ", " << state.start.lat << ")" << std::endl;
  std::cout << "  End: (" << state.end.lon << ", " << state.end.lat << ")" << std::endl;
  std::cout << "  Distance: " << state.distance << " km" << std::endl;

  std::cout << "  Route: " << std::endl;
  for (const auto& loc : state.route) {
    std::cout << "    (" << loc.lon << ", " << loc.lat << ")" << std::endl;
  }
}

void writeBootState(const BootState& state) {
  FILE* file = fopen(MODE_FILE, "w");
  if (!file) {
    LOG("Failed to open", MODE_FILE, "for writing");
    return;
  }
  fprintf(file, "%c,%c,%.6f,%.6f,", state.version, state.mode, state.center.lon, state.center.lat);

  fprintf(file, "%d,", state.zoom);

  fprintf(file, "%.6f,%.6f,", state.start.lon, state.start.lat);
  fprintf(file, "%.6f,%.6f,", state.end.lon, state.end.lat);

  fprintf(file, "%.6f,", state.distance);

  for (const auto& loc : state.route) {
    fprintf(file, "%.6f,%.6f,", loc.lon, loc.lat);
  }

  fclose(file);
  LOG("Boot state written to", MODE_FILE, "for mode:", modeToString(state.mode));
}

BootState readBootState() {
  BootState state = {
    CURRENT_BM_VER,
    ModeMap,
    {INIT_LON, INIT_LAT},
    INIT_ZOOM,
    {0.0f, 0.0f},
    {0.0f, 0.0f},
    {},
    0.0f
  };
  FILE* file = fopen(MODE_FILE, "r");
  if (!file) {
    LOG("Failed to open", MODE_FILE, "for reading");
    return state;
  }

  char versionChar;
  if (fscanf(file, "%c,", &versionChar) == 1) {
    if (versionChar != CURRENT_BM_VER) {
      fclose(file);
      LOGF("Incorrect version of boot state: %d", versionChar);
      if (remove(MODE_FILE) == 0) {
        LOG("File", MODE_FILE, "successfully deleted");
      } else {
        LOG("Failed to delete", MODE_FILE);
      }
      return state;
    }
  }

  char modeChar;
  if (fscanf(file, "%c,", &modeChar) == 1) {
    state.mode = static_cast<Mode>(modeChar);
  }
  LOG("Current mode:", modeToString(state.mode));

  fscanf(file, "%f,%f,", &state.center.lon, &state.center.lat);

  fscanf(file, "%d,", &state.zoom);

  fscanf(file, "%f,%f,", &state.start.lon, &state.start.lat);
  fscanf(file, "%f,%f,", &state.end.lon, &state.end.lat);

  fscanf(file, "%f,", &state.distance);

  float lon, lat;
  while (fscanf(file, "%f,%f,", &lon, &lat) == 2) {
    state.route.push_back({lon, lat});
  }

  fclose(file);
  LOG("Boot state read from", MODE_FILE);
  printBootState(state);
  return state;
}
