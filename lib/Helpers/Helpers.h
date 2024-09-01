#ifndef HELPERS_H
#define HELPERS_H

#include <globals.h>
#include <vector>

struct CenterAndZoom {
  Location center;
  int zoom;
};

struct BBox {
  float minLon;
  float maxLon;
  float minLat;
  float maxLat;
};

BBox getBBox(const std::vector<Location>& locations);
CenterAndZoom getBBoxCenterAndZoom(const BBox& bbox);

#endif //HELPERS_H
