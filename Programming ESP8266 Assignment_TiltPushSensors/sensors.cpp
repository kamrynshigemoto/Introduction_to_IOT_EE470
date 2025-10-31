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
