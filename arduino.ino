#define ARDUINOJSON_ENABLE_ARDUINO_STRING 0

#include <Wire.h>
#include <ArduinoJson.h>
//#include <TinyGPS++.h>
//#include <NeoSWSerial.h>
//#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

//=========================================================
/*
 * Rui Santos 
 * Complete Project Details https://randomnerdtutorials.com
 */

//static const int RXPin = 12, TXPin = 11;
//static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
//TinyGPSPlus gps;

// The serial connection to the GPS device
//NeoSWSerial ss(RXPin, TXPin);

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connect to the Arduino digital pin 11
#define ONE_WIRE_BUS 11

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
//=========================================================

// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3

#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance
EnergyMonitor emon2;
EnergyMonitor emon3;

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Message buffer
char line0[21];
char line1[21];
char line2[21];
char line3[21];

// Create json document instance
StaticJsonDocument<512> doc;

// Relay config
#define relayCB_R 4
#define relayCB_S 6
#define relayCB_T 7
#define pumpRelay 8
#define fanRelay 9
//#define resetButton 2

// Program state pin config
#define stateButton 3

// Function prototype
//void clearRelay();
//void changeState();
void updateState();
void runMonitoring();
void runControlling();
void getInfoFromSerial();
bool arrayFindGreaterThan(float val);
bool arrayFindLowerThan(float val);

// Variable for state
bool AUTO_STATE = true;
bool MANUAL_STATE = false;

void setup()
{
  // Start serial communication for debugging purposes
  Serial.begin(9600);
  
  // initialize the LCD
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.print("Initializing...");
  
  // Start serial communication with gps
  //ss.begin(GPSBaud);
  // Start up the dallas library
  sensors.begin();
  
  emon1.voltage(A0, 147, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon2.voltage(A1, 150, 1.7);
  emon3.voltage(A2, 150, 1.7);
  emon1.current(A3, 0.7);       // Current: input pin, calibration.
  emon2.current(A6, 1);
  emon3.current(A7, 0.7);

  // Relay init
  pinMode(relayCB_R, OUTPUT);
  pinMode(relayCB_S, OUTPUT);
  pinMode(relayCB_T, OUTPUT);
  pinMode(pumpRelay, OUTPUT);
  pinMode(fanRelay, OUTPUT);
  digitalWrite(relayCB_R, HIGH);
  digitalWrite(relayCB_S, HIGH);
  digitalWrite(relayCB_T, HIGH);
  digitalWrite(pumpRelay, HIGH);
  digitalWrite(fanRelay, HIGH);
  //pinMode(resetButton, INPUT_PULLUP);

  // Program state init
  pinMode(stateButton, INPUT_PULLUP);

  // initialize command array
  doc["cmd"][0] = 0;
  doc["cmd"][1] = 0;

  // Setup interrupt
  //attachInterrupt(digitalPinToInterrupt(resetButton), clearRelay, FALLING);
  //attachInterrupt(digitalPinToInterrupt(stateButton), changeState, CHANGE);
}

void loop()
{ 
  getInfoFromSerial();
  runMonitoring();
  runControlling();
  updateState();
  delay(100);
}

void runMonitoring() {
  emon1.calcVI(10,40);         // Calculate all. No.of half wavelengths (crossings), time-out
  emon2.calcVI(10,40);
  emon3.calcVI(10,40);
  //emon1.serialprint();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)
  
  //float realPower       = emon1.realPower;        //extract Real Power into variable
  //float apparentPower   = emon1.apparentPower;    //extract Apparent Power into variable
  //float powerFActor     = emon1.powerFactor;      //extract Power Factor into Variable
  //float supplyVoltage   = emon1.Vrms;             //extract Vrms into Variable
  //float Irms            = emon1.Irms;             //extract Irms into Variable
  
  char UL1_str[4];
  char CT1_str[4];
  char UL2_str[4];
  char CT2_str[4];
  char UL3_str[4];
  char CT3_str[4];
  
  dtostrf((emon1.Vrms < 20) ? 0 : emon1.Vrms, 3, 0, UL1_str);
  dtostrf((emon2.Vrms < 20) ? 0 : emon2.Vrms + 0.3, 3, 0, UL2_str);
  dtostrf((emon3.Vrms < 20) ? 0 : emon3.Vrms + 0.5, 3, 0, UL3_str);
  dtostrf(emon1.Irms, 3, 1, CT1_str);
  dtostrf(emon2.Irms + 0.03, 3, 1, CT2_str);
  dtostrf(emon3.Irms + 0.04, 3, 1, CT3_str);
  
  sprintf(line0, "I R %3s S %3s T %3s", CT1_str, CT2_str, CT3_str);
  sprintf(line1, "U R %3s S %3s T %3s", UL1_str, UL2_str, UL3_str);

  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures();

  float Temp1 = sensors.getTempCByIndex(0) + 1.2;
  float Temp2 = sensors.getTempCByIndex(1) + 1.9;
  char Temp1_str[5];
  char Temp2_str[5];

  dtostrf(Temp1, 4, 1, Temp1_str);
  dtostrf(Temp2, 4, 1, Temp2_str);

  sprintf(line2, "T1 %6s T2 %6s", Temp1_str, Temp2_str);
  
  lcd.setCursor(0,0);
  lcd.print(line0);
  
  lcd.setCursor(0,1);
  lcd.print(line1);

  lcd.setCursor(0,2);
  lcd.print(line2);
  
  lcd.setCursor(0,3);
  lcd.print(line3);

  doc["current"][0] = emon1.Irms;
  doc["current"][1] = emon2.Irms + 0.03;
  doc["current"][2] = emon3.Irms + 0.04;
  doc["voltage"][0] = (emon1.Vrms < 20) ? 0 : emon1.Vrms;
  doc["voltage"][1] = (emon2.Vrms < 20) ? 0 : emon2.Vrms + 0.3;
  doc["voltage"][2] = (emon3.Vrms < 20) ? 0 : emon3.Vrms + 0.5;
  doc["temperature"][0] = Temp1;
  doc["temperature"][1] = Temp2;

  serializeJson(doc, Serial);
}

void runControlling() {
  if ((AUTO_STATE) &&
             (arrayFindGreaterThan(0.8) || (doc["temperature"][0] > 45.0) || (doc["temperature"][1] > 45.0))
             ) {
    digitalWrite(fanRelay, HIGH);
    digitalWrite(pumpRelay, HIGH);
    digitalWrite(relayCB_R, LOW); delay(100);
    digitalWrite(relayCB_S, LOW); delay(100);
    digitalWrite(relayCB_T, LOW); delay(100);
  } else if ((AUTO_STATE) &&
             (arrayFindGreaterThan(0.6) || (doc["temperature"][0] > 40.0) || (doc["temperature"][1] > 40.0))
             ) {
    digitalWrite(fanRelay, LOW);
    digitalWrite(pumpRelay, LOW);
  } else if ((AUTO_STATE) &&
             (arrayFindGreaterThan(0.3) || (doc["temperature"][0] > 35.0) || (doc["temperature"][1] > 35.0))
             ) {
    digitalWrite(fanRelay, HIGH);
    digitalWrite(pumpRelay, LOW);
  } else if (AUTO_STATE){
    digitalWrite(fanRelay, HIGH);
    digitalWrite(pumpRelay, HIGH);
  }
}

void getInfoFromSerial() {
  // check serial buffer from nodeMCU
  if (Serial.available() > 0) {
    deserializeJson(doc, Serial);

    char lat_str[10];
    char lng_str[11];

    dtostrf(doc["location"]["lat"], 9, 6, lat_str);
    dtostrf(doc["location"]["lng"], 10, 6, lng_str);
    sprintf(line3, "%9s,%10s", lat_str, lng_str);

    if (MANUAL_STATE) {
      digitalWrite(pumpRelay, (doc["cmd"][0] == 1) ? LOW : HIGH);
      digitalWrite(fanRelay, (doc["cmd"][1] == 1) ? LOW : HIGH);
    }
  }
}

bool arrayFindGreaterThan(float val) {
  for (uint8_t i = 0; i < 3; i++) {
    if (doc["current"][i] > val) return true;
  }

  return false;
}

bool arrayFindLowerThan(float val) {
  for (uint8_t i = 0; i < 3; i++) {
    if (doc["current"][i] < val) return true;
  }

  return false;
}

void clearRelay() {
  digitalWrite(relayCB_R, HIGH);
  digitalWrite(relayCB_S, HIGH);
  digitalWrite(relayCB_T, HIGH);
  digitalWrite(pumpRelay, HIGH);
  digitalWrite(fanRelay, HIGH);
}

/*
void changeState() {
  if (digitalRead(stateButton)) {
    AUTO_STATE = true;
    MANUAL_STATE = false;
  } else {
    AUTO_STATE = false;
    MANUAL_STATE = true;
  }
  digitalWrite(relayCB_R, HIGH);
  digitalWrite(relayCB_S, HIGH);
  digitalWrite(relayCB_T, HIGH);
  digitalWrite(pumpRelay, HIGH);
  digitalWrite(fanRelay, HIGH);
}
*/

void updateState() {
  if (!digitalRead(stateButton) && AUTO_STATE) {
    AUTO_STATE = false;
    MANUAL_STATE = true;
    clearRelay();
  } else if (digitalRead(stateButton) && MANUAL_STATE) {
    AUTO_STATE = true;
    MANUAL_STATE = false;
    clearRelay();
  }
}
