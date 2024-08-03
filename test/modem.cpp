 #define TINY_GSM_MODEM_A7
#include <TinyGsmClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

char ssid[] = "SSID"; // your SSID
char pass[] = "Password"; // your SSID Password

// Your GPRS credentials
const char apn[] = "cit-moi";
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <HardwareSerial.h>
#define DEBUG true
#define BAUD_RATE 115200
#include<stdio.h>
#include <ArduinoJson.h>
#define A9G_SIZE_RX 1024 // used in A9G.setRxBufferSize()
#define data_BUFFER_LENGTH 1024
/***********************************/
#define A9G_PON 13 //ESP32 GPIO13 A9G POWON
#define A9G_RST 12 //ESP32 GPIO12 A9G RESET
#define A9G_WAKE 27 //ESP32 GPIO27 A9G WAKE
#define A9G_LOWP 14 //ESP32 GPIO14 A9G ENTER LOW POWER MODULE
#define A9G_TXD 35 //ESP32 GPIO35(RX), A9G TXD
#define A9G_RXD 32 //ESP32 GPIO32(TX), A9G RXD

int A9GPOWERON();

String response = "";
HardwareSerial A9G(1);
int WiFi_Count = 10;
String payload;
unsigned long previousMillis = 0;
unsigned long interval = 30000;

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(A9G, Serial);
TinyGsm modem(debugger);
#else
// Initialize GSM modem
TinyGsm modem(A9G);
#endif

// Initialize GSM client
TinyGsmClient client(modem);
// Set to true, if modem is connected
bool modemConnected = false;

const char server[] = "arduino.cc";
const char resource[] = "/";
const int port = 80; // port 80 is the default for HTTP

void setup() {

Serial.begin(115200); /* Define baud rate for A9G communication */
A9G.begin(9600, SERIAL_8N1, A9G_TXD, A9G_RXD); // RX and TX of ES32
A9G.setRxBufferSize(A9G_SIZE_RX);

WiFi.begin(ssid, pass);
Serial.println("");
Serial.print("Connecting to WiFi");
// Wait for connection

while (WiFi.status() != WL_CONNECTED) {
if (WiFi_Count > 0) {
delay(500);
Serial.print(".");
WiFi_Count = WiFi_Count - 1;
}
else goto exit;
}
//If connection successful show IP address in serial monitor
exit:
if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
Serial.println("");
Serial.print("Connected to ");
Serial.println(ssid);
Serial.print("WiFi Signals:");
Serial.println(WiFi.RSSI());
Serial.print("WIFI Local IP:");
Serial.print(WiFi.localIP()); //IP address assigned to your ESP
}

Serial.println("\nA9G Test Started...");

for (char ch = ' '; ch <= 'z'; ch++) {
A9G.write(ch);
}
A9G.println("");
delay(500);
// sendData("AT+RST=1", 1000, DEBUG); //Software reset
// delay(500);

/************************************/
pinMode(A9G_PON, OUTPUT);//LOW LEVEL ACTIVE
pinMode(A9G_RST, OUTPUT);//HIGH LEVEL ACTIVE
pinMode(A9G_LOWP, OUTPUT);//LOW LEVEL ACTIVE

digitalWrite(A9G_PON, HIGH); // Keep A9G OFF(HIGH) initially
digitalWrite(A9G_RST, LOW);
digitalWrite(A9G_LOWP, HIGH);

Serial.println("Turning ON A9G...");
Serial.println();

if (A9GPOWERON() == 1)
{
Serial.println("A9G POWER ON.");
Serial.println();
}

String modemInfo = modem.getModemInfo();
Serial.print(F("Modem: "));
Serial.println(modemInfo);

String modemIIMEI = modem.getIMEI();
Serial.print(F("IMEI: "));
Serial.println(modemIIMEI);
}

void loop() {
HTTPClient http;
if (!modemConnected) {
Serial.print(F("Waiting for network..."));
if (!modem.waitForNetwork()) {
Serial.println(" fail");
delay(1000);
return;
}
Serial.println(" OK");

Serial.print(F("Connecting to "));
Serial.print(apn);
if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail");
    delay(1000);
    return;
}

modemConnected = true;
Serial.println(" Success!");
}

// if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
// HTTPWiFi_Data();
// Serial.println("WiFi Data");
// JSON_Data();
// }

if(client.connect(server, port)) {
Serial.println("Connected to server");
// Make a HTTP request:
client.print("GET ");
client.print(resource);
client.println(" HTTP/1.0");
client.println();
} else {
Serial.println("connection failed");
}
}

void HTTPWiFi_Data() {

HTTPClient http; //Declare an object of class HTTPClient

String Rec_URL = "http://apk.vehtechs.com/api/imei?imei=123456789456789";

Serial.println(Rec_URL);
Serial.println();

http.begin(Rec_URL); //Specify request destination
int httpResponseCode = http.GET(); //Send the request

if (httpResponseCode > 0) { //Check the returning code
Serial.print("HTTP Response code: ");
Serial.println(httpResponseCode);
payload = http.getString(); //Get the request response payload
Serial.println(payload); //Print the response payload
Serial.println(); Serial.println();
}
else {
Serial.print("Error code: ");
Serial.println(httpResponseCode);
}
http.end(); //Close connection
}

void JSON_Data() {
StaticJsonDocument<512> doc;
DeserializationError error = deserializeJson(doc, payload);

if (error) {
Serial.print("deserializeJson() failed: ");
Serial.println(error.c_str());
//return;
}
else {
const char* isArm = doc["isArm"]; // "1"
const char* wifi = doc["wifi"]; // "0"
const char* wifi_password = doc["wifi_password"]; // nullptr
const char* wifi_connssid = doc["wifi_connssid"]; // "0"
int wifi_disconnect = doc["wifi_disconnect"]; // 0
const char* wifi_disc_name = doc["wifi_disc_name"]; // nullptr
int engine = doc["engine"]; // 1
const char* phoneNumber = doc["phoneNumber"]; // "542351685"
const char* number1 = doc["number1"]; // nullptr
const char* number2 = doc["number2"]; // nullptr
const char* number3 = doc["number3"]; // nullptr
const char* number4 = doc["number4"]; // nullptr
const char* number5 = doc["number5"]; // nullptr
const char* gyro = doc["gyro"]; // "0"
const char* gps = doc["gps"]; // "1"
const char* powerSupply = doc["powerSupply"]; // "1"
const char* plateNo = doc["plateNo"]; // "32568"
Serial.print("isArm= ");
Serial.println(isArm);
Serial.print("wifi= ");
Serial.println(wifi);
Serial.print("wifi_password= ");
Serial.println(wifi_password);
Serial.print("wifi_connssid= ");
Serial.println(wifi_connssid);
Serial.print("wifi_disconnect= ");
Serial.println(wifi_disconnect);
Serial.print("wifi_disc_name= ");
Serial.println(wifi_disc_name);
Serial.print("engine= ");
Serial.println(engine);
Serial.print("phoneNumber= ");
Serial.println(phoneNumber);
Serial.print("number1= ");
Serial.println(number1);
Serial.print("number2= ");
Serial.println(number2);
Serial.print("number3= ");
Serial.println(number3);
Serial.print("number4= ");
Serial.println(number4);
Serial.print("number5= ");
Serial.println(number5);
Serial.print("gyro= ");
Serial.println(gyro);
Serial.print("gps= ");
Serial.println(gps);
Serial.print("powerSupply= ");
Serial.println(powerSupply);
Serial.print("plateNo= ");
Serial.println(plateNo);
Serial.println(); Serial.println(); Serial.println();
}
delay(500);
}

String sendData(String command, const int timeout, boolean debug)
{
String response = "";
Serial.print("Command Sent: "); Serial.println(command);
A9G.println(command);

unsigned long time = millis();
while (A9G.available() == 0) {}

while (A9G.available())
{
char c = A9G.read();
response += c;
// Serial.print(c);
delay(5);
}

if (debug)
{
Serial.println();
Serial.print("A9G Response: ");
Serial.println(response);
}

if (response.indexOf('{') > 0) {
int l = response.indexOf('{');
String res = response.substring(l);
// Serial.println(res);
response = res;
}
return response;
}

int A9GPOWERON() //Send Command to A9G to Turn ON
{
digitalWrite(A9G_PON, LOW);
delay(3000);
digitalWrite(A9G_PON, HIGH);
delay(5000);
String msg = String("");
label:
msg = sendData("AT", 2000, DEBUG);
if ( msg.indexOf("OK") >= 0 ) {
Serial.println("GET OK");
Serial.println();
return 1;
}
else {
Serial.println("NOT GET OK");
Serial.println();
goto label; //If NOT GET OK GOTO label
return 0;
}
}