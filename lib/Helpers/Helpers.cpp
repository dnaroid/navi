#include "Helpers.h"


BBox getBBox(const std::vector<Location>& locations) {
  float minLon = std::numeric_limits<float>::max();
  float maxLon = std::numeric_limits<float>::lowest();
  float minLat = std::numeric_limits<float>::max();
  float maxLat = std::numeric_limits<float>::lowest();

  for (const auto& loc : locations) {
    if (loc.lon < minLon) minLon = loc.lon;
    if (loc.lon > maxLon) maxLon = loc.lon;
    if (loc.lat < minLat) minLat = loc.lat;
    if (loc.lat > maxLat) maxLat = loc.lat;
  }
  return {minLon, maxLon, minLat, maxLat};
}

CenterAndZoom getBBoxCenterAndZoom(const BBox& bbox) {
  const int WORLD_DIM = 256;
  int zoom;

  auto latRad = [](float lat) -> float {
    float sin = std::sin(lat * M_PI / 180.0f);
    double radX2 = std::log((1 + sin) / (1 - sin)) / 2;
    return std::max(std::min(radX2, M_PI), -M_PI) / 2;
  };

  auto zoom_lambda = [](float mapPx, float worldPx, float fraction) -> int {
    return std::floor(std::log(mapPx / worldPx / fraction) / M_LN2);
  };

  float latFraction = (latRad(bbox.maxLat) - latRad(bbox.minLat)) / M_PI;
  float lngDiff = bbox.maxLon - bbox.minLon;
  float lngFraction = ((lngDiff < 0) ? (lngDiff + 360.0f) : lngDiff) / 360.0f;

  int latZoom = zoom_lambda(SCREEN_HEIGHT, WORLD_DIM, latFraction);
  int lngZoom = zoom_lambda(SCREEN_WIDTH, WORLD_DIM, lngFraction);

  zoom = std::min(latZoom, lngZoom);
  zoom = std::min(zoom, ZOOM_MAX);

  return {
    .center = {
      .lon = (bbox.minLon + bbox.maxLon) / 2.0f,
      .lat = (bbox.minLat + bbox.maxLat) / 2.0f,
    },
    .zoom = zoom
  };
}
