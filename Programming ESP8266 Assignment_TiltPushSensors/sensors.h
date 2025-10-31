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
