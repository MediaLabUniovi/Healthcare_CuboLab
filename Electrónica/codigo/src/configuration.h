#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFiMulti.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <time.h>
#include <math.h>
#include <WebServer.h>
#include <LittleFS.h>

#include "updateLab.h"
#include "serverLab.h"
#include "cuboFunctions.h"

#define led_r 25
#define led_g 26
#define led_b 27

extern int chargingState;
extern WiFiMulti wifiMulti;
extern Preferences preferences;
extern unsigned long startTime;
extern boolean lowBattery;
extern Adafruit_MPU6050 mpu;


#define BUZZER_PIN 18
#define MPU_INT_PIN 14 
#define CHARGE_PIN 33  