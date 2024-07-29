#ifndef GEOCODE_H
#define GEOCODE_H


const char* apiUrl = "https://api.openrouteservice.org/pgeocode/search";
const char* apiKey2 = "5b3ce3597851110001cf62489b0f664f6a754381a7074b32b6d7a3b6";
// const char* apiKey = "5b3ce3597851110001cf6248fbb6a43a404c4389be12a7b1d58a30af";


const char* queryText = "hallera%20130";
const float minLon = 18.341674804687504;
const float minLat = 54.22229214016528;
const float maxLon = 18.86558532714844;
const float maxLat = 54.523871119721406;


class Geocode {
public:
  Geocode() {
  }


  Location search(char* text, Location center) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String url = String(apiUrl) +
        "?api_key=" + apiKey2 +
        "&text=" + urlEncode(text) +
        "&size=20" +
        "&focus.point.lon=" + String(center.lon, 6) +
        "&focus.point.lat=" + String(center.lat, 6) +
        "&boundary.rect.min_lon=" + String(minLon, 6) +
        "&boundary.rect.min_lat=" + String(minLat, 6) +
        "&boundary.rect.max_lon=" + String(maxLon, 6) +
        "&boundary.rect.max_lat=" + String(maxLat, 6) +
        "&layers=address,venue,neighbourhood";

      http.begin(url);

      http.addHeader("accept", "*/*");
      http.addHeader("accept-language", "en-US,en;q=0.9,ru;q=0.8,be;q=0.7,pl;q=0.6");
      http.addHeader("content-type", "application/json");
      http.addHeader("origin", "https://maps.openrouteservice.org");
      http.addHeader("referer", "https://maps.openrouteservice.org/");
      http.addHeader("sec-ch-ua", "\"Not)A;Brand\";v=\"99\", \"Google Chrome\";v=\"127\", \"Chromium\";v=\"127\"");
      http.addHeader("sec-ch-ua-mobile", "?0");
      http.addHeader("sec-ch-ua-platform", "\"macOS\"");
      http.addHeader("sec-fetch-dest", "empty");
      http.addHeader("sec-fetch-mode", "cors");
      http.addHeader("sec-fetch-site", "same-site");
      http.addHeader("user-agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36");

      String payload;

      int httpCode = http.GET();

      if (httpCode > 0) {
        payload = http.getString();
        Serial.println(payload);
      } else {
        Serial.println("Error on HTTP request");
        http.end();
        return Location{0, 0};;
      }
      http.end();

      DynamicJsonDocument doc(-1);
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return Location{0, 0};;
      }

      JsonArray coords = doc["features"][0]["geometry"]["coordinates"];
      Location loc;
      loc.lon = coords[0].as<float>();
      loc.lat = coords[1].as<float>();

      return loc;
    }
    Serial.println("WiFi not connected");
    return Location{0, 0};
  }

  String urlEncode(const String& str) {
    String encoded = "";
    char c;
    for (size_t i = 0; i < str.length(); i++) {
      c = str.charAt(i);
      if (c == ' ') {
        encoded += "+";
      } else if (c == '!') {
        encoded += "%21";
      } else if (c == '#') {
        encoded += "%23";
      } else if (c == '$') {
        encoded += "%24";
      } else if (c == '%') {
        encoded += "%25";
      } else if (c == '&') {
        encoded += "%26";
      } else if (c == '\'') {
        encoded += "%27";
      } else if (c == '(') {
        encoded += "%28";
      } else if (c == ')') {
        encoded += "%29";
      } else if (c == '*') {
        encoded += "%2A";
      } else if (c == '+') {
        encoded += "%2B";
      } else if (c == ',') {
        encoded += "%2C";
      } else if (c == '-') {
        encoded += "%2D";
      } else if (c == '.') {
        encoded += "%2E";
      } else if (c == '/') {
        encoded += "%2F";
      } else if (c >= '0' && c <= '9') {
        encoded += c;
      } else if (c == ':') {
        encoded += "%3A";
      } else if (c == ';') {
        encoded += "%3B";
      } else if (c == '<') {
        encoded += "%3C";
      } else if (c == '=') {
        encoded += "%3D";
      } else if (c == '>') {
        encoded += "%3E";
      } else if (c == '?') {
        encoded += "%3F";
      } else if (c == '@') {
        encoded += "%40";
      } else if (c == '[') {
        encoded += "%5B";
      } else if (c == '\\') {
        encoded += "%5C";
      } else if (c == ']') {
        encoded += "%5D";
      } else if (c == '^') {
        encoded += "%5E";
      } else if (c == '_') {
        encoded += "%5F";
      } else if (c == '`') {
        encoded += "%60";
      } else if (c == '{') {
        encoded += "%7B";
      } else if (c == '|') {
        encoded += "%7C";
      } else if (c == '}') {
        encoded += "%7D";
      } else if (c == '~') {
        encoded += "%7E";
      } else {
        encoded += c;
      }
    }
    return encoded;
  }

private :
  Location location;
};

#endif //GEOCODE_H
