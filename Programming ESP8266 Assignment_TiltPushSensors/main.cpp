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

----------------------------------------------------------------------------------------------------------------------------
sensors.cpp code
//--------------------------------------------------
//Purpose: Defines sensor functions for ESP8266 circuit that sends data to my database.
//         Reads output from temp/hum sensor and sends that data with timestamp when either
//         tilt switch or button is activated. Sends data as node_1 or node_2
//Inputs: Digital input from tilt switch (D6) and push button (D5), temp/hum readings (D7), timezone from timeapi.io
//Outputs: Messages for debugging and HTTP GET request to send data
//Date: October 22, 2025
//Compiler: Platformio
//Author: Kamryn Shigemoto
//Versions:
//          V1 - original version
//--------------------------------------------------
//File Dependencies:
//--------------------------------------------------
//"sensors.h" header file, "ESP8266HTTPClient.h" for HTTP requests, "ESP8266WiFi.h" for WiFi connection, "DHTesp.h" for DHT11 (temp/hum sensor)
//--------------------------------------------------
//Main Program
//--------------------------------------------------

#include "sensors.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <DHTesp.h>

const int switchPin = D5;
const int tiltPin = D6;
const int dhtPin = D7;

// set up DHT11 sensor
DHTesp dht;

void setup_dht() {
    dht.setup(dhtPin, DHTesp::DHT11);
}

// check if push button activated and send data as node_1
void check_switch(String timeZone) {
    if (digitalRead(switchPin) == LOW) {
        Serial.println("Node 1 triggered");
        float s1 = read_sensor_1();     //temp
        float s2 = read_sensor_2();     //hum
        String timestamp = read_time(timeZone);
        if (check_error(1, timestamp, s1, s2)) {
            transmit(1, timestamp, s1, s2);
        }
    }
}

// check if tilt switch activated and send data as node_2
void check_tilt(String timeZone) {
    if (digitalRead(tiltPin) == HIGH) {
        Serial.println("Node 2 triggered");
        float s1 = read_sensor_1();     //temp
        float s2 = read_sensor_2();     //hum
        String timestamp = read_time(timeZone);
        if (check_error(2, timestamp, s1, s2)) {
            transmit(2, timestamp, s1, s2);
        }
    }
}

// read temperature using sensor
float read_sensor_1() {
    return dht.getTemperature();
}

// read humidity using sensor
float read_sensor_2() {
    return dht.getHumidity();
}

// get time from timeapi.io
String read_time(String timeZone) {
    HTTPClient https;       //make https request to get time data
    String url = "https://timeapi.io/api/Time/current/zone?timeZone=" + timeZone;
    WiFiClientSecure client;
    client.setInsecure();   //disable certificate verification (safe for testing)
    https.begin(client, url);
    https.setTimeout(10000);  // 10 seconds
    
    //send GET request and wait 1s before retrying because it takes time to load
    int httpCode = https.GET();
    if (httpCode <= 0) {
        delay(1000);
        httpCode = https.GET();  // Retry once
    }

    // get the JSON response and parse to get dateTime string
    String timestamp = "";

    if (httpCode == 200) {
        String payload = https.getString();
        int index = payload.indexOf("\"dateTime\":\"");
        if (index != -1) {
            index += 12;    //move past the "dateTime":" part
            int endIndex = payload.indexOf("\"", index);
            if (endIndex != -1) {
                timestamp = payload.substring(index, endIndex);
            }
        }
    }
    else {
        Serial.println("Failed to get time. HTTP code: " + String(httpCode));
    }

    https.end();
    return timestamp;
}

// make sure all parameters are within range, valid, and included before being sent over
bool check_error(int node_ID, String timeReceived, float s1, float s2) {
    if (timeReceived == "") return false;
    if (isnan(s1) || isnan(s2)) return false;
    if (s1 < -10 || s1 > 100) return false;
    if (s2 < 0 || s2 > 100) return false;
    return true;
}

// send data to database by building URL based on parameters
bool transmit(int node_ID, String timeReceived, float s1, float s2) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    String nodeName = (node_ID == 1) ? "node_1" : "node_2";
    String url = "https://kshigemotoee.com/db_insert.php?nodeID=" + nodeName + "&timeReceived=" + timeReceived + "&nodeTemp=" + String(s1) + "&humidity=" + String(s2);
    
    timeReceived.replace(" ", "%20");  // Encode spaces
    Serial.println("Sending to URL: " + url);
    http.begin(client, url);
    int httpCode = http.GET();      
    Serial.println("HTTP code: " + String(httpCode));   //debugging stuff because of previous errors
    Serial.println("Redirected to: " + http.getLocation());


    if (httpCode > 0) {
        String response = http.getString();
        Serial.println("Server response: " + response);
    }
    else {
        Serial.println("Error sending data: " + String(httpCode));
    }

    http.end();
    return httpCode == 200;
}

----------------------------------------------------------------------------------------------------------------------------
sensors.h code
//--------------------------------------------------
//Purpose: Declare functions to be used in sensors.cpp file
//Inputs: Time zone, inputs from push button and tilt switch
//Outputs: URL string to be transmitted and readings from sensor
//Date: October 22, 2025
//Compiler: Platformio
//Author: Kamryn Shigemoto
//Versions:
//          V1 - original version
//--------------------------------------------------
//File Dependencies:
//--------------------------------------------------
//"Arduino.h" to access Arduino Files, "sensors.cpp" runs these functions
//--------------------------------------------------
//Main Program
//--------------------------------------------------

#ifndef SENSORS_H
#define SENSORS_H
#include <Arduino.h>

extern const int switchPin;
extern const int tiltPin;

// declare functions to be defined in sensors.cpp file
void setup_dht();
void check_switch(String timeZone);     // checks if switch pressed than node_1 is sending data
void check_tilt(String timeZone);       // check if tilt sensor activate, then node_2 is sending data
String read_time(String timeZone);      // extract time data from timeapi.io
float read_sensor_1();                  //read data from temperature sensor
float read_sensor_2();                  //read data from humidity sensor
bool transmit(int node_ID, String timeReceived, float s1, float s2);       // transmit data to website
bool check_error(int node_ID, String timeReceived, float s1, float s2);    // check data for errors before sending


#endif
