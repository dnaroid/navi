#include <esp_wifi.h>
#include <globals.h>
#include <Secrets.h>
#include <WebServer.h>

#define CONTENT_TYPE "image/jpeg"


void handle_jpg_stream() {
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);

  while (client.connected()) {
    cam.run();
    if (!client.connected()) break;

    response = "--frame\r\n";
    response += "Content-Type: image/jpeg\r\n\r\n";
    server.sendContent(response);

    client.write((char*)cam.getfb(), cam.getSize());
    server.sendContent("\r\n");
  }
}

void handle_jpg_file() {
  WiFiClient client = server.client();

  cam.run();
  if (!client.connected()) {
    return;
  }
  // server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
  server.sendHeader("Content-Type", CONTENT_TYPE);
  server.sendContent("\r\n");
  client.write((char*)cam.getfb(), cam.getSize());
}

void handle_jpg() {
  WiFiClient client = server.client();

  cam.run();
  if (!client.connected()) {
    return;
  };
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-disposition: inline; filename=capture.jpg\r\n";
  response += "Content-type: image/jpeg\r\n\r\n";
  server.sendContent(response);
  client.write((char*)cam.getfb(), cam.getSize());
}

void handleNotFound() {
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(404, "text/plain", message);
}

void setupServer() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(1);
  WiFi.softAP(ssid_name, ssid_password);
  IPAddress ip = WiFi.softAPIP();
  Serial.println(F("Access Point started"));
  Serial.println("");
  Serial.println(ip);

  server.on("/", HTTP_GET, handle_jpg_stream);
  server.on("/jpg", HTTP_GET, handle_jpg);
  server.on("/file", HTTP_GET, handle_jpg_file);
  server.onNotFound(handleNotFound);
  server.begin(80);

  LOG("HTTP server started");
}

void loopServer() {
  server.handleClient();
}
