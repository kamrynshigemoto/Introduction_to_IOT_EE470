main.cpp code
//--------------------------------------------------
//Purpose: This program will connect ESP8266 to WiFi (hotspot) and ask user to input timezone.
//         It will check if tilt switch or push button sensor activated and then send temp and 
//         humidity data to database with timestamps.
//Inputs: Selected time zone, digital input from switches, temperature and humidity from sensor
//Outputs: MAC Address, selected time zone, HTTP url data, debugging/status updates
//Date: October 22, 2025
//Compiler: Platformio
//Author: Kamryn Shigemoto
//Versions:
//          V1 - original version
//--------------------------------------------------
//File Dependencies:
//--------------------------------------------------
// "sensors.h" header file, "Arduino.h" for Arduino functions, "ESP8266Wifi.h" to connect to WiFi
//--------------------------------------------------
//Main Program
//--------------------------------------------------
#include <Arduino.h>
#include <ESP8266Wifi.h>
#include "sensors.h"

#define wifi_ssid "--"
#define wifi_password "--"

//setup to select time zone
const char* timeZones[] = {
  "America/New_York",     //1 - Eastern
  "America/Chicago",      //2 - Central
  "America/Denver",       //3 - Mountain
  "America/Los_Angeles",  //4 - Pacific (default)
  "America/Anchorage",    //5 - Alaska
  "Pacific/Honolulu",     //6 - Hawaii-Aleutian
  "America/Puerto_Rico"   //7 - Atlantic
};

//default time zone 
String selectedTimeZone = "America/Los_Angeles";

void setup () {
  Serial.begin(9600);
  pinMode(switchPin, INPUT_PULLUP);   // use internal pull-up resistor in button module
  pinMode(tiltPin, INPUT);
  setup_dht();

  //connect to wifi network (my hotspot)
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected");
  
  // find mac add
  Serial.println(WiFi.macAddress());

  Serial.println("Select Time Zone (the default is set to America/Los-Angeles):");
  Serial.println("Enter a number (1-7) or press Enter to use default:");
  Serial.println("1: Eastern (New York)");
  Serial.println("2: Central (Chicago)");
  Serial.println("3: Mountain (Denver)");
  Serial.println("4: Pacific (Los Angeles)");
  Serial.println("5: Alaska (Anchorage)");
  Serial.println("6: Hawaii-Aleutian (Honolulu)");
  Serial.println("7: Atlantic (Puerto Rico)");
  
  //read input from user and clean it up 
  while (Serial.available() == 0) delay(100);
  String input = Serial.readStringUntil('\n');
  input.trim();

  //if user entered input that's valid and within range then set as time zone
  if (input.length() > 0) {
    int choice = input.toInt();
    if (choice >= 1 && choice <= 7) {
      selectedTimeZone = timeZones[choice - 1];
    }
  }

  Serial.println("Selected Time Zone: " + selectedTimeZone);
}

// continuosly check if switch pressed, tilt switch activated
void loop() {
  check_switch(selectedTimeZone);
  check_tilt(selectedTimeZone);
  delay(1000);
}
