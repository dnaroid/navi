#ifndef A9G_H
#define A9G_H

#include "WiFi.h"
#define DEBUG true


//******************* Pin Configurations *******************//

#define SOS_Button 7


//******************* Necessary Variables *******************//
boolean stringComplete = false;
String inputString = "";
String fromGSM = "";
bool CALL_END = 1;
char* response = " ";
String res = "";
int c = 0;
String msg;
String custom_message;

//******************* SIM Paramaters *******************//


String SOS_NUM = "+48514411576";


//******************* SOS Button Press  *******************//
int SOS_Time = 5; // Press the button 5 sec

class A9G {
public:
  explicit A9G(const HardwareSerial& serial): gpsSerial(serial), isReady(false) {
  }

  void setup() {
    gpsSerial.begin(115200, SERIAL_8N1, A9G_RX, A9G_TX);
    pinMode(A9G_PON, OUTPUT); //LOW LEVEL ACTIVE
    // pinMode(A9G_RST, OUTPUT); //HIGH LEVEL ACTIVE
    pinMode(A9G_LOWP, OUTPUT); //LOW LEVEL ACTIVE

    // digitalWrite(A9G_RST, LOW);
    digitalWrite(A9G_LOWP, HIGH);
    digitalWrite(A9G_PON, HIGH);
    delay(1000);
    digitalWrite(A9G_PON, LOW);
    delay(10000);

    // Making Radio OFF for power saving
    WiFi.mode(WIFI_OFF); // WiFi OFF
    btStop(); // Bluetooth OFF

    // pinMode(SOS_Button, INPUT_PULLUP);
    pinMode(A9G_LOWP, OUTPUT);

    return;

    // Waiting for A9G to setup everything for 20 sec
    delay(20000);

    digitalWrite(A9G_LOWP, LOW); // Sleep Mode OFF

    // Just Checking
    print("->64:A9G.h Just Checking:");
    msg = "";
    msg = sendData("AT", 1000, DEBUG);
    while (msg.indexOf("OK") == -1) {
      msg = sendData("AT", 1000, DEBUG);
      Serial.print(".");
    }
    Serial.println("");

    // Turning ON GPS
    print("->73:A9G.h Turning ON GPS:");
    msg = "";
    msg = sendData("AT+GPS=1", 2000, DEBUG);
    while (msg.indexOf("OK") == -1) {
      msg = sendData("AT+GPS=1", 1000, DEBUG);
      Serial.print(".");
    }
    Serial.println("");

    // GPS low power
    print("->83:A9G.h GPS low power:");
    msg = "";
    msg = sendData("AT+GPSLP = 2", 2000, DEBUG);
    while (msg.indexOf("OK") == -1) {
      msg = sendData("AT+GPSLP = 2", 1000, DEBUG);
      Serial.print(".");
    }
    Serial.println("");

    // Configuring Sleep Mode to 1
    print("->92:A9G.h Configuring Sleep Mode to 1:");
    msg = "";
    msg = sendData("AT+SLEEP = 1", 2000, DEBUG);
    while (msg.indexOf("OK") == -1) {
      msg = sendData("AT+SLEEP = 1", 1000, DEBUG);
      Serial.print(".");
    }
    Serial.println("");

    /*
        // For SMS
        print("->101:A9G.h For SMS:");
        msg = "";
        msg = sendData("AT+CMGF = 1", 2000, DEBUG);
        while (msg.indexOf("OK") == -1) {
          msg = sendData("AT+CMGF = 1", 1000, DEBUG);
          Serial.print(".");
        }
        Serial.println("");

        msg = "";
        msg = sendData("AT+CSMP  = 17,167,0,0 ", 2000, DEBUG);
        while (msg.indexOf("OK") == -1) {
          msg = sendData("AT+CSMP  = 17,167,0,0 ", 1000, DEBUG);
          Serial.print(".");
        }
        Serial.println("");

        msg = "";
        msg = sendData("AT+CPMS = \"SM\",\"ME\",\"SM\" ", 2000, DEBUG);
        while (msg.indexOf("OK") == -1) {
          msg = sendData("AT+CPMS = \"SM\",\"ME\",\"SM\" ", 1000, DEBUG);
          Serial.print(".");
        }
        Serial.println("");
    */

    /*
    // For Speaker
    print("->125:A9G.h For Speaker:");
    msg = "";
    msg = sendData("AT+SNFS=2", 2000, DEBUG);
    while (msg.indexOf("OK") == -1) {
      msg = sendData("AT+SNFS=2", 1000, DEBUG);
      Serial.print(".");
    }
    Serial.println("");

    msg = "";
    msg = sendData("AT+CLVL=8", 2000, DEBUG);
    while (msg.indexOf("OK") == -1) {
      msg = sendData("AT+CLVL=8", 1000, DEBUG);
      Serial.print(".");
    }
    Serial.println("");
    */

    // A9G_Ready_msg(); // Sending Ready Msg to SOS Number

    digitalWrite(A9G_LOWP, HIGH); // Sleep Mode ON

    Serial.println("GSM/GPS done");
  }

  void initGPS() {
    // Turning ON GPS
    print("->73:A9G.h Turning ON GPS:");
    msg = "";
    msg = sendData("AT+GPS=1", 2000, DEBUG);
    while (msg.indexOf("OK") == -1) {
      msg = sendData("AT+GPS=1", 1000, DEBUG);
      Serial.print(".");
    }
    Serial.println("");

    // GPS low power
    // print("->83:A9G.h GPS low power:");
    // msg = "";
    // msg = sendData("AT+GPSLP = 2", 2000, DEBUG);
    // while (msg.indexOf("OK") == -1) {
    //   msg = sendData("AT+GPSLP = 2", 1000, DEBUG);
    //   Serial.print(".");
    // }
    // Serial.println("");
  }

  void loop(bool gps) {
    {
      if (isReady && gps) {
        getGPS();
      }
      //listen from GSM Module
      if (gpsSerial.available()) {
        char inChar = gpsSerial.read();


        if (inChar == '\n') {
          if (fromGSM == "READY\r" && !isReady) {
            initGPS();
            isReady = true;
          }

          //check the state
          if (fromGSM == "SEND LOCATION\r" || fromGSM == "send location\r" || fromGSM == "Send Location\r") {
            Get_gmap_link(0); // Send Location without Call
            digitalWrite(A9G_LOWP, HIGH); // Sleep Mode ON
          }

          //check the state
          else if (fromGSM == "BATTERY?\r" || fromGSM == "battery?\r" || fromGSM == "Battery?\r") {
            digitalWrite(A9G_LOWP, LOW); // Sleep Mode OFF
            Serial.println("---------Battery Status-------");
            msg = "";
            msg = sendData("AT+CBC?", 2000, DEBUG);
            while (msg.indexOf("OK") == -1) {
              msg = sendData("AT+CBC?", 1000, DEBUG);
              Serial.print(".");
            }
            Serial.println("");

            msg = msg.substring(19, 24);
            response = &msg[0];

            Serial.print("Recevied Data - ");
            Serial.println(response); // printin the String in lower character form
            Serial.println("\n");


            custom_message = response;
            Send_SMS(custom_message);
          }

          // For Auto Call Recieve
          else if (fromGSM == "RING\r") {
            digitalWrite(A9G_LOWP, LOW); // Sleep Mode OFF
            Serial.println("---------ITS RINGING-------");
            gpsSerial.println("ATA");
          } else if (fromGSM == "NO CARRIER\r") {
            Serial.println("---------CALL ENDS-------");
            CALL_END = 1;
            digitalWrite(A9G_LOWP, HIGH); // Sleep Mode ON
          }

          //write the actual response
          Serial.println(fromGSM);
          //clear the buffer
          fromGSM = "";
        } else {
          fromGSM += inChar;
        }
        delay(20);
      }

      // read from port 0, send to port 1:
      if (Serial.available()) {
        int inByte = Serial.read();
        gpsSerial.write(inByte);
      }

      // When SOS button is pressed
      // if (digitalRead(SOS_Button) == LOW && CALL_END == 1) {
      //   Serial.print("Calling In.."); // Waiting for 5 sec
      //   for (c = 0; c < SOS_Time; c++) {
      //     Serial.println((SOS_Time - c));
      //     delay(1000);
      //     if (digitalRead(SOS_Button) == HIGH) break;
      //   }
      //
      //   if (c == 5) {
      //     Get_gmap_link(1); // Send Location with Call
      //   }
      //
      //   //only write a full message to the GSM module
      //   if (stringComplete) {
      //     gpsSerial.print(inputString);
      //     inputString = "";
      //     stringComplete = false;
      //   }
      // }
    }
    delay(100);
  }

  void getGPS() {
    print("->281:A9G.h getGPS:");
    digitalWrite(A9G_LOWP, LOW);
    delay(1000);
    gpsSerial.println("AT+LOCATION = 2");
    Serial.println("AT+LOCATION = 2");

    while (!gpsSerial.available());
    while (gpsSerial.available()) {
      char add = gpsSerial.read();
      res = res + add;
      delay(1);
    }

    res = res.substring(17, 38);
    response = &res[0];

    Serial.print("Recevied Data - ");
    Serial.println(response); // printin the String in lower character form
    Serial.println("\n");

    if (strstr(response, "GPS NOT")) {
      Serial.println("No Location data");
    } else {
      int i = 0;
      while (response[i] != ',') i++;

      String location = (String)response;
      String lat = location.substring(2, i);
      String longi = location.substring(i + 1);
      Serial.println(lat);
      Serial.println(longi);
    }
  }

  //---------------------------------------------  Getting Location and making Google Maps link of it. Also making call if needed
  void Get_gmap_link(bool makeCall) {
    digitalWrite(A9G_LOWP, LOW);
    delay(1000);
    gpsSerial.println("AT+LOCATION = 2");
    Serial.println("AT+LOCATION = 2");

    while (!gpsSerial.available());
    while (gpsSerial.available()) {
      char add = gpsSerial.read();
      res = res + add;
      delay(1);
    }

    res = res.substring(17, 38);
    response = &res[0];

    Serial.print("Recevied Data - ");
    Serial.println(response); // printin the String in lower character form
    Serial.println("\n");

    if (strstr(response, "GPS NOT")) {
      Serial.println("No Location data");
      //------------------------------------- Sending SMS without any location
      custom_message = "Unable to fetch location. Please try again";
      Send_SMS(custom_message);
    } else {
      int i = 0;
      while (response[i] != ',') i++;

      String location = (String)response;
      String lat = location.substring(2, i);
      String longi = location.substring(i + 1);
      Serial.println(lat);
      Serial.println(longi);

      String Gmaps_link = "I'm Here " + ("http://maps.google.com/maps?q=" + lat + "+" + longi); //http://maps.google.com/maps?q=38.9419+-78.3020
      //------------------------------------- Sending SMS with Google Maps Link with our Location


      custom_message = Gmaps_link;
      Send_SMS(custom_message);
    }
    response = "";
    res = "";
    if (makeCall) {
      Serial.println("Calling Now");
      gpsSerial.println("ATD" + SOS_NUM);
      CALL_END = 0;
    }
  }

  void A9G_Ready_msg() {
    custom_message = "A9G Ready!!";
    Send_SMS(custom_message);
  }

  String sendData(String command, const int timeout, boolean debug) {
    String temp = "";
    gpsSerial.println(command);
    long int time = millis();
    while ((time + timeout) > millis()) {
      while (gpsSerial.available()) {
        char c = gpsSerial.read();
        temp += c;
      }
    }
    if (debug) {
      Serial.print(temp);
    }
    return temp;
  }


  void Send_SMS(String message) {
    //for (int i = 0; i < Total_Numbers; i++)
    {
      gpsSerial.println("AT+CMGF=1");
      delay(1000);
      gpsSerial.println("AT+CMGS=\"" + SOS_NUM + "\"\r");
      delay(1000);

      gpsSerial.println(message);
      delay(1000);
      gpsSerial.println((char)26);
      delay(1000);

      gpsSerial.println("AT+CMGD=1,4"); // delete stored SMS to save memory
      delay(3000);
    }
  }

  HardwareSerial gpsSerial;
  bool isReady;
};


#endif //A9G_H
