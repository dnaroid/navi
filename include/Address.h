#ifndef ADDRESS_H
#define ADDRESS_H

#include "globals.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


struct SearchResult {
  Location location;
  String name;
};

class Address {
public:
  Address() {
  }

  std::vector<SearchResult> getResults() const {
    return results;
  }

  bool search(const String& address, Location myLoc) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = "https://nominatim.openstreetmap.org/search.php?q=gdansk%20" + address +
        "&viewbox=18.40347%2C54.44648%2C18.91090%2C54.27804" +
        "&bounded=1" +
        "&limit=" + ADDR_SEARCH_LIMIT +
        "&format=jsonv2";
      http.begin(url);
      int httpCode = http.GET();
      String payload = "{}";
      if (httpCode > 0) {
        payload = http.getString();
        // Serial.println(payload);
      } else {
        Serial.println("Error on HTTP request");
        return false;
      }
      http.end();

      DynamicJsonDocument doc(-1);
      const DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return false;
      }

      JsonArray array = doc.as<JsonArray>();

      for (JsonVariant result : array) {
        SearchResult res;
        res.location = Location{result["lon"].as<float>(), result["lat"].as<float>()};
        res.name = result["display_name"].as<String>();
        results.push_back(res);
      }

      results.clear();
      std::sort(results.begin(), results.end(), [myLoc](const SearchResult& a, const SearchResult& b) {
        return coord::haversineDistance(myLoc, a.location) < coord::haversineDistance(myLoc, b.location);
      });

      return !results.empty();
    }
    Serial.println("WiFi not connected");
    return false;
  }

private:
  std::vector<SearchResult> results;
};


#endif //ADDRESS_H
