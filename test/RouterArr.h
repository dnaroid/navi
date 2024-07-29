#ifndef ROUTE_H
#define ROUTE_H

#define CYCLING "cycling"
#define DRIVING "driving"
#define WALKING "walking"
#define MAX_ROUTE_POINTS 1000
#define MAX_RESPONSE_SIZE 20000


class Router {
public:
  Router() : distance(0), duration(0), route(nullptr), routeSize(0) {
  }

  ~Router() {
    if (route != nullptr) delete[] route;
  }

  bool search(Location start, Location end, const char* profile) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = String("http://router.project-osrm.org/route/v1/") + profile + "/" +
        String(start.lon, 6) + "," + String(start.lat, 6) + ";" +
        String(end.lon, 6) + "," + String(end.lat, 6) +
        "?overview=full&geometries=geojson";

      http.begin(url);
      int httpCode = http.GET();

      if (httpCode > 0) {
        String payload = http.getString();
        http.end();

        DynamicJsonDocument doc(MAX_RESPONSE_SIZE);
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.f_str());
          return false;
        }

        JsonArray coordinates = doc["routes"][0]["geometry"]["coordinates"];
        distance = doc["routes"][0]["distance"].as<float>();
        duration = doc["routes"][0]["duration"].as<float>();
        routeSize = coordinates.size();

        if (route != nullptr) delete[] route;

        route = new Location[routeSize];

        for (size_t i = 0; i < routeSize; i++) {
          route[i].lon = coordinates[i][0].as<float>();
          route[i].lat = coordinates[i][1].as<float>();
        }

        return true;
      } else {
        Serial.println("Error on HTTP request");
        http.end();
        return false;
      }
    } else {
      Serial.println("WiFi not connected");
      return false;
    }
  }

  Location* getRoute() const {
    return route;
  }

  size_t getRouteSize() const {
    return routeSize;
  }

  float getDistance() const {
    return distance;
  }

  float getDuration() const {
    return duration;
  }

private:
  Location* route;
  size_t routeSize;
  float distance;
  float duration;
};

#endif //ROUTE_H
