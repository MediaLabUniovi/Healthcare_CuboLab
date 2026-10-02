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

// LEDs legacy (hardware actual sin LEDs; se mantienen por compatibilidad)
#define led_r 4
#define led_g 26
#define led_b 2

extern int chargingState;
extern WiFiMulti wifiMulti;
extern Preferences preferences;
extern unsigned long startTime;
extern boolean lowBattery;
extern Adafruit_MPU6050 mpu;


// DFPlayer + alimentación con TPS61023
#define ENABLE_DFPLAYER 1
#define DFPLAYER_ENABLE_PIN 17
#define DFPLAYER_UART_RX_PIN 27  // TX del ESP32 (conectar a RX del DFPlayer)
#define DFPLAYER_UART_TX_PIN 25  // RX del ESP32 (conectar a TX del DFPlayer)
#define DFPLAYER_POWER_STABILIZE_MS 4000
#define DFPLAYER_INIT_RETRIES 3
#define DFPLAYER_AUTO_POWER_OFF_MS 60000
#define DFPLAYER_SLEEP_WAIT_MS 10000

// Pistas MP3 para eventos (carpeta /mp3)
#define TRACK_EVENT_MOVEMENT 1
#define TRACK_EVENT_TELEMETRY_OK 1
#define TRACK_EVENT_WIFI_ERROR 2

// UART con microcontrolador externo.
// Si el cableado final invierte TX/RX, intercambiar estos dos defines.
#define AUX_UART_TX_PIN 32
#define AUX_UART_RX_PIN 15
#define AUX_UART_BAUDRATE 115200

// Protocolo UART con base (ESP32-C3 + OLED)
#define CHARGE_DEBOUNCE_MS 150
#define DOCK_HANDSHAKE_RETRIES 3
#define DOCK_HELLO_TIMEOUT_MS 300
#define DOCK_PING_INTERVAL_MS 2000
#define DOCK_ACK_TIMEOUT_MS 250

// I2C MPU6050
#define MPU_SDA_PIN 22
#define MPU_SCL_PIN 21
#define MPU_INT_PIN 14

// Batería / carga
#define BATTERY_ADC_PIN 34
#define CHARGE_PIN 35
#define CHARGE_ACTIVE_LEVEL HIGH
#define CHARGE_INACTIVE_LEVEL LOW

// Botón de configuración (cerrado a masa)
#define CONFIG_BUTTON_PIN 33
#define CONFIG_BUTTON_ACTIVE_LEVEL LOW

// Conversión ADC -> voltaje real de batería (divisor 1:2)
#define ADC_REF_VOLTAGE 3.3f
#define ADC_MAX_READING 4095.0f
#define BATTERY_DIVIDER_RATIO 2.0f
#define BATTERY_CALIBRATION_FACTOR 1.0f
#define BATTERY_VOLTAGE_MIN 3.30f
#define BATTERY_VOLTAGE_MAX 4.20f
