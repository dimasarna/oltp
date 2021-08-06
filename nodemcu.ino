#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include <ArduinoJson.h>

/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on NodeMCU.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right NodeMCU module
  in the Tools -> Board menu!

  For advanced settings please follow ESP examples :
   - ESP8266_Standalone_Manual_IP.ino
   - ESP8266_Standalone_SmartConfig.ino
   - ESP8266_Standalone_SSL.ino

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
//#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp8266.h>

#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
//char auth[] = "DzTHs6XogNtZmyENzzGDzVMkgUI6I3ld";
char auth[] = "jFjEeW3BiAIEDbnXEUmJ_0aQCin01gX9";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

// Create json document instance
StaticJsonDocument<512> doc;
StaticJsonDocument<256> data;

// Create instance blynk widget
WidgetLCD lcd(V1);
WidgetMap hwMap(V2);

// Message buffer
char line0[17];
char line1[17];
char line2[17];
char line3[17];

BlynkTimer timer;
int timer1_ID;
int timer2_ID;

void printVI()
{
  lcd.clear();
  lcd.print(0, 0, line0);
  lcd.print(0, 1, line1);
  timer.toggle(timer1_ID);
  timer.toggle(timer2_ID);
}

void printTL()
{
  lcd.clear();
  lcd.print(0, 0, line2);
  lcd.print(0, 1, line3);
  timer.toggle(timer1_ID);
  timer.toggle(timer2_ID);
}

BLYNK_WRITE(V0) {
  float latitude = param[0].asFloat();
  float longitude = param[1].asFloat();
  char lat_str[8];
  char lng_str[9];
  dtostrf(latitude, 7, 4, lat_str);
  dtostrf(longitude, 8, 4, lng_str);
  sprintf(line3, "%6s,%7s", lat_str, lng_str);
  hwMap.location(0, latitude, longitude, "Machine");

  data["location"]["lat"] = latitude;
  data["location"]["lng"] = longitude;
  serializeJson(data, Serial);
}

BLYNK_WRITE(V6) {
  data["cmd"][0] = param.asInt();
  serializeJson(data, Serial);
}

BLYNK_WRITE(V7) {
  data["cmd"][1] = param.asInt();
  serializeJson(data, Serial);
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  wifiMulti.addAP("SSID_TA_PLN", "12345678");   // add Wi-Fi networks you want to connect to
  wifiMulti.addAP("ANOTHER_SSID", "ANOTHER_PASSWORD");   // add Wi-Fi networks you want to connect to

  //Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.config(auth, "blynk.iot-cm.com", 8080);
  Blynk.config(auth, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(500);
  }

  Blynk.connect();
  timer.setTimeout(3600000L, [] () {} ); // dummy/sacrificial Function
  timer1_ID = timer.setInterval(5000L, printVI);
  timer2_ID = timer.setInterval(5000L, printTL);
  timer.disable(timer2_ID);
}

void loop()
{
  Blynk.run();
  timer.run();
  if (Serial.available()) {
    deserializeJson(doc, Serial);
    
    float UL1 = doc["voltage"][0];
    float CT1 = doc["current"][0];
    float UL2 = doc["voltage"][1];
    float CT2 = doc["current"][1];
    float UL3 = doc["voltage"][2];
    float CT3 = doc["current"][2];
  
    char UL1_str[4];
    char CT1_str[4];
    char UL2_str[4];
    char CT2_str[4];
    char UL3_str[4];
    char CT3_str[4];
  
    dtostrf(UL1, 3, 0, UL1_str);
    dtostrf(UL2, 3, 0, UL2_str);
    dtostrf(UL3, 3, 0, UL3_str);
    dtostrf(CT1, 3, 1, CT1_str);
    dtostrf(CT2, 3, 1, CT2_str);
    dtostrf(CT3, 3, 1, CT3_str);
  
    sprintf(line0, " %3sA %3sA %3sA ", CT1_str, CT2_str, CT3_str);
    sprintf(line1, " %3sV %3sV %3sV ", UL1_str, UL2_str, UL3_str);

    float Temp1 = doc["temperature"][0];
    float Temp2 = doc["temperature"][1];
    char Temp1_str[5];
    char Temp2_str[5];

    dtostrf(Temp1, 4, 1, Temp1_str);
    dtostrf(Temp2, 4, 1, Temp2_str);
  
    sprintf(line2, "T1 %4s T2 %4s", Temp1_str, Temp2_str);
  }
}
