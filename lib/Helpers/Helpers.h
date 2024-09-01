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
void bbox_reset();
void bbox_compare(Location loc);
BBox bbox_result();

#endif //HELPERS_H
