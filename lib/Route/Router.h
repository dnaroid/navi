#ifndef ROUTE_H
#define ROUTE_H

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

  bool search(Location start, Location end, String profile) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String url = "https://api.openrouteservice.org/v2/directions/" + profile + "/geojson";

      http.begin(url);
      http.addHeader("accept", "*/*");
      http.addHeader("accept-language", "en-US,en;q=0.9,ru;q=0.8,be;q=0.7,pl;q=0.6");
      http.addHeader("authorization", ROUTER_API_KEY);
      http.addHeader("content-type", "application/json");
      http.addHeader("origin", "https://maps.openrouteservice.org");
      http.addHeader("priority", "u=1, i");
      http.addHeader("referer", "https://maps.openrouteservice.org/");
      http.addHeader("sec-ch-ua", "\"Not)A;Brand\";v=\"99\", \"Google Chrome\";v=\"127\", \"Chromium\";v=\"127\"");
      http.addHeader("sec-ch-ua-mobile", "?0");
      http.addHeader("sec-ch-ua-platform", "\"macOS\"");
      http.addHeader("sec-fetch-dest", "empty");
      http.addHeader("sec-fetch-mode", "cors");
      http.addHeader("sec-fetch-site", "same-site");
      http.addHeader("user-agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36");

      DynamicJsonDocument jsonDoc(-1);
      JsonArray coordinates = jsonDoc.createNestedArray("coordinates");
      JsonArray startPoint = coordinates.createNestedArray();
      startPoint.add(start.lon);
      startPoint.add(start.lat);
      JsonArray endPoint = coordinates.createNestedArray();
      endPoint.add(end.lon);
      endPoint.add(end.lat);

      jsonDoc["elevation"] = true;
      jsonDoc["instructions_format"] = "html";
      JsonArray extraInfo = jsonDoc.createNestedArray("extra_info");
      extraInfo.add("surface");
      extraInfo.add("steepness");
      extraInfo.add("waytype");
      jsonDoc["language"] = "en";
      jsonDoc["units"] = "km";
      jsonDoc["preference"] = "recommended";

      String payload;
      serializeJson(jsonDoc, payload);

      int httpCode = http.POST(payload);

      if (httpCode > 0) {
        payload = http.getString();
        // Serial.println(payload);
      } else {
        Serial.println("Error on HTTP request");
        http.end();
        return false;
      }
      http.end();

      DynamicJsonDocument doc(-1);
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return false;
      }

      JsonArray coords = doc["features"][0]["geometry"]["coordinates"];
      distance = doc["features"][0]['properties']['summary']["distance"].as<float>();
      duration = doc["features"][0]['properties']['summary']["duration"].as<float>();

      route.clear();
      for (JsonVariant coord : coords) {
        route.push_back(Location{coord[0].as<float>(), coord[1].as<float>()});
      }

      return !route.empty();
    }
    Serial.println("WiFi not connected");
    return false;
  }

  const String BIKE = "cycling-regular";
  const String CAR = "car";
  const String FOOT = "foot";

private :
  std::vector<Location> route;
  float distance;
  float duration;
};

#endif //ROUTE_H
