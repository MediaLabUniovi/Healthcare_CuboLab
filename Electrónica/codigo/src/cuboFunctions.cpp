#include "cuboFunctions.h"
#include "dfPlayerLab.h"
#include <driver/rtc_io.h>

boolean networksAvailable = false;
String networks = "";

float tolerance = 1;  // Rango de tolerancia para comparar los valores de los ejes

float ax_values[6];  // Para almacenar ax de cada lado
float ay_values[6];  // Para almacenar ay de cada lado
float az_values[6];  // Para almacenar az de cada lado

int minBatt = 5950; // mV de batería mínima
int maxBatt = 8200; // mV de batería máxima

bool isChargingActive() {
  return digitalRead(CHARGE_PIN) == CHARGE_ACTIVE_LEVEL;
}

bool isConfigButtonPressed() {
  return digitalRead(CONFIG_BUTTON_PIN) == CONFIG_BUTTON_ACTIVE_LEVEL;
}

void getLimits() {
  /* Recuperar los valores de los ejes x, y , z de la memoria no volatil para cada lado del cubo después de un reseteo */
  for (int sd = 0; sd < 6; sd++) {
    // Generar los nombres de las claves para cada eje y lado
    String s_x = "s" + String(sd) + "_x";
    String s_y = "s" + String(sd) + "_y";
    String s_z = "s" + String(sd) + "_z";

    // Recuperar los valores desde Preferences para cada eje
    ax_values[sd] = preferences.getFloat(s_x.c_str(), 0.0);
    ay_values[sd] = preferences.getFloat(s_y.c_str(), 0.0);
    az_values[sd] = preferences.getFloat(s_z.c_str(), 0.0);
  }
}

int determineCubeSide(float ax, float ay, float az) {
    /* Determinar en qué lado del cubo se encuentra */
    for (int side = 0; side < 6; side++) {
        // Comparar el valor de ax, ay y az con los valores almacenados para el lado actual dentro de una tolerancia de ±0.5
        if (ax > (ax_values[side] - tolerance) && ax < (ax_values[side] + tolerance) &&
            ay > (ay_values[side] - tolerance) && ay < (ay_values[side] + tolerance) &&
            az > (az_values[side] - tolerance) && az < (az_values[side] + tolerance)) {
            return side;  // Si está dentro del rango, devolver el lado actual
        }
    }

    /*Serial.print("AX: ");
    Serial.print(ax);
    Serial.print(" AY: ");
    Serial.print(ay);
    Serial.print(" AZ: ");
    Serial.println(az);*/
    //Serial.println("No se puede determinar el lado del cubo.");

    // Si no se encuentra un lado, aumentar la tolerancia en 0.5 y volver a intentar
    tolerance = tolerance + 0.5;
    return -1;   
}

boolean connectWiFi(){
  //wifiMulti.addAP("WifiCube", "M3d14L4b2024_");
  wifiMulti.addAP("MediaLab guest", "medialab2019");

  // Busca las redes WiFi almacenadas en la memoria no volátil para conectarse a alguna de ellas, cuando lo consigue imprime la dirección IP 
  for (int i = 1; i <= 5; i++) {
    // Construir las claves de búsqueda para el ssid y el password
    String ssidKey = "ssid" + String(i);
    String passKey = "pass" + String(i);

    // Obtener el ssid y el password correspondientes al índice actual
    String ssid = preferences.getString(ssidKey.c_str(), "");
    String password = preferences.getString(passKey.c_str(), "");
    Serial.println(ssid);
    Serial.println(password);
    // Si no se encontraron ssid y password para esta clave, continuar con el siguiente índice
    if (ssid == "" || password == "") {
      continue;
    }

    // Convertir los SSID y contraseñas a const char*
    const char* ssidChar = ssid.c_str();
    const char* passwordChar = password.c_str();

    // Agregar la red WiFi con su contraseña
    wifiMulti.addAP(ssidChar, passwordChar);
  }

  int attempts  = 0;
  while (attempts  < 20) {

    if (wifiMulti.run() == WL_CONNECTED) {
      Serial.print("Conectado a la red WiFi");
      Serial.print(WiFi.SSID());
      Serial.print(" Dirección IP: ");
      Serial.println(WiFi.localIP());
      break; // Sal del bucle si la conexión tiene éxito
    } 
    
    else {
      attempts ++;
      Serial.println("Conexión fallida. Intento nuevamente...");
    }

    
    delayLab(5000); // Espera un momento para la conexión
  }

  if (attempts  >= 9) {
    // To-Do: Realiza alguna acción si no se puede conectar después de 10 intentos
    Serial.println("No se pudo conectar a la red WiFi después de 10 intentos.");

    // Hardware nuevo sin LEDs locales: se mantiene solo feedback por audio.
    playEventTrack(TRACK_EVENT_WIFI_ERROR);
    delayLab(300);
    delayLab(300);

    playEventTrack(TRACK_EVENT_WIFI_ERROR);
    delayLab(500);
    delayLab(500);

    playEventTrack(TRACK_EVENT_WIFI_ERROR);
    delayLab(1000);
    delayLab(500);


    return false;
  }

  return true;
}

void ledsOff(){
  // Hardware actual sin LEDs en el dispositivo principal.
}

void delayLab(long wait){
  // Funcion para reemplazar delay
  long tt =millis();
  while((millis()-tt)<wait){}
}

void sendHMI(){
  // En arquitectura de doble micro, la interfaz visual vive en la base.
  // El audio queda reservado para confirmación de servidor en telemetryLab.
}


void configHMI(){
  // Interfaz local desactivada: la base con OLED mostrará el estado de configuración.
}

void chargingHMI(){
  // Interfaz local desactivada: la base con OLED mostrará el estado de carga.
}

void testLowBattery(){
  // Sin LED local en esta revisión de hardware.
}

int getBattery(){
  // Promediar lecturas para reducir ruido del ADC en deep-sleep wakeups.
  const int kSamples = 16;
  uint32_t rawAccum = 0;
  uint32_t mvAccum = 0;

  for (int i = 0; i < kSamples; i++) {
    rawAccum += analogRead(BATTERY_ADC_PIN);
    mvAccum += analogReadMilliVolts(BATTERY_ADC_PIN);
  }

  float adcRaw = rawAccum / (float)kSamples;
  float adcVoltage = (mvAccum / (float)kSamples) / 1000.0f;

  // Fallback si la calibracion por mV no estuviera disponible.
  if (adcVoltage <= 0.01f) {
    adcVoltage = (adcRaw * ADC_REF_VOLTAGE) / ADC_MAX_READING;
  }

  float batteryVoltage = adcVoltage * BATTERY_DIVIDER_RATIO * BATTERY_CALIBRATION_FACTOR;

  float batt = (batteryVoltage - BATTERY_VOLTAGE_MIN) * 100.0f / (BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN);

  /*Serial.print("Battery Read: ");
  Serial.println((int)roundf(adcRaw));
  Serial.print("Battery Voltage: ");
  Serial.println(batteryVoltage, 3);*/

  if (batt < 0) {
    batt = 0;
  }
  if (batt > 100) {
    batt = 100;
  }

  return (int)batt;
}

void goToSleep(){
  dfPlayerPowerOff();

    if(lowBattery){
      Serial.println("Low Battery");
      delay(500);

    }

    // Entrar en modo deep sleep para ahorrar batería
    Serial.println("Entrando en modo de bajo consumo.");

    // Wake por movimiento/carga (alto) y por botón de configuración (bajo).
    esp_sleep_enable_ext1_wakeup((1ULL << MPU_INT_PIN) | (1ULL << CHARGE_PIN), ESP_EXT1_WAKEUP_ANY_HIGH);
    rtc_gpio_pullup_en((gpio_num_t)CONFIG_BUTTON_PIN);
    rtc_gpio_pulldown_dis((gpio_num_t)CONFIG_BUTTON_PIN);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)CONFIG_BUTTON_PIN, 0);
    esp_deep_sleep_start();  // Entrar en modo deep sleep
}


void scanNetworks()
{
  networksAvailable = false;

  Serial.println("Detectando WiFis cercanas");



  int nNetworks = WiFi.scanNetworks();

  for (int i = 0; i < nNetworks; i++)
  {
    String ssidd = WiFi.SSID(i);
    networks += "<option value='" + ssidd + "'>" + ssidd + "</option>";

  }


  networksAvailable = true;
}

float mean(float readings[], int size) {
  // Función para calcular la media de 10 lecturas
  float sum = 0.0;
  for (int i = 0; i < size; i++) {
    sum += readings[i];
  }
  return sum / size;
}