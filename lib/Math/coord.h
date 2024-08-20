#ifndef COORD_H
#define COORD_H

class coord {
public:
  static Point locationToScreen(Location loc, Location centerLoc, int zoom) {
    Point centerPixels = locationToPixels(centerLoc, zoom);
    Point locPixels = locationToPixels(loc, zoom);
    int screenX = locPixels.x - centerPixels.x + SCREEN_CENTER_X;
    int screenY = locPixels.y - centerPixels.y + SCREEN_CENTER_Y;
    return Point{screenX, screenY};
  }

  static Point locationToPixels(Location loc, int zoom) {
    int n = 1 << zoom;
    int x = static_cast<int>((loc.lon + 180.0) / 360.0 * n * TILE_SIZE);
    float radLat = loc.lat * M_PI / 180.0;
    int y = static_cast<int>((1 - std::log(std::tan(radLat) + 1 / std::cos(radLat)) / M_PI) / 2 * n * TILE_SIZE);
    return Point{x, y};
  }

  static Location pixelsToLocation(Point pixels, int zoom) {
    int n = 1 << zoom;
    float lon = pixels.x / static_cast<float>(n * TILE_SIZE) * 360.0 - 180.0;
    float lat_rad = std::atan(std::sinh(M_PI * (1 - 2.0 * pixels.y / static_cast<float>(n * TILE_SIZE))));
    float lat = lat_rad * 180.0 / M_PI;
    return Location{lon, lat};
  }

  // static Location pointToLocation(Pos point, Location cursorLoc, int zoom) {
  //   Point centerPixels = locationToPixels(cursorLoc, zoom);
  //   int pixelX = point.x + centerPixels.x - SCREEN_CENTER_X;
  //   int pixelY = point.y + centerPixels.y - SCREEN_CENTER_Y;
  //   return pixelsToLocation(Point{pixelX, pixelY}, zoom);
  // }

  static double haversineDistance(Location loc1, Location loc2) {
    const double R = 6371;
    double dLat = radians(loc2.lat - loc1.lat);
    double dLon = radians(loc2.lon - loc1.lon);
    double a = sin(dLat / 2) * sin(dLat / 2) +
      cos(radians(loc1.lat)) * cos(radians(loc2.lat)) *
      sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
  }
};

#endif // COORD_H
