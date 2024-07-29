#ifndef ROUTE_H
#define ROUTE_H

#define CYCLING "cycling"
#define DRIVING "driving"
#define WALKING "walking"
#define MAX_RESPONSE_SIZE 20000


class Router {
public:
  Router() : distance(0), duration(0) {
  }

  std::vector<Location> getRoute() const {
    return route;
  }

  float getDistance() const {
    return distance;
  }

  float getDuration() const {
    return duration;
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

      String payload = "{}";
      if (httpCode > 0) {
        payload = http.getString();
        print("->41:Router.h payload:", payload);
      } else {
        Serial.println("Error on HTTP request");
        http.end();
        return false;
      }
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

      route.clear();
      for (JsonVariant coord : coordinates) {
        Location loc;
        loc.lon = coord[0].as<float>();
        loc.lat = coord[1].as<float>();
        route.push_back(loc);
      }

      return true;
    }
    Serial.println("WiFi not connected");
    return false;
  }

private:
  std::vector<Location> route;
  float distance;
  float duration;
};

#endif //ROUTE_H
