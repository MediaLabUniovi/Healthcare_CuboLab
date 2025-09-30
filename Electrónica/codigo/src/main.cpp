#include <Arduino.h>
#include "configuration.h"


Adafruit_MPU6050 mpu;

const float accelThreshold = 0.05;  // Umbral para detectar movimiento en g
const float gyroThreshold = 0.1;    // Umbral para detectar rotación en rad/s

// Opciones de configuración del MPU6050
const float motion = 1; 
const float motionDuration = 1.0;

boolean modoDemo = false;
int side = 0;  // Variable para rastrear el lado del cubo (de 0 a 5)

boolean calibration = true;

HTTPClient http;

int chargingState; // Estado de carga del dispositivo (0: no cargando, 1: cargando, 2: cargado)
WiFiMulti wifiMulti; // Instancia de WiFiMulti para conectarse a varias redes WiFi
Preferences preferences;// Instancia de Preferences para almacenar los valores de los ejes en memoria no volátil
unsigned long startTime;

int battery;
bool lowBattery = false;
bool moving = true;

bool configFirst = true;


void setup() {
    Serial.begin(115200);
    if (!mpu.begin(0x68)) {
        Serial.println("No se pudo encontrar un MPU6050.");
        while (1) {
            delay(10);
        }
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.println("MPU6050 inicializado correctamente.");


    pinMode(MPU_INT_PIN, INPUT_PULLUP);
    pinMode(CHARGE_PIN, INPUT_PULLUP);


    preferences.begin("MediaLab", false);

    getLimits(); // Recuperar los valores de los ejes x, y, z para cada lado del cubo

    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());

    // Configurar el MPU6050 para activar la interrupción en movimiento
    mpu.setMotionInterrupt(true);
    mpu.setMotionDetectionThreshold(motion);         
    mpu.setMotionDetectionDuration(motionDuration);

    if (analogRead(CHARGE_PIN)>1000){
      // Función para indicar que se está cargando el cubo
        Serial.println("Dispositivo despertado por voltaje positivo en el pin 33.");
        chargingState = 1;
        startTime = millis();
    }

    battery = getBattery();
    Serial.println(battery);


    if (battery<15){
      lowBattery=true;
      gpio_hold_dis(GPIO_NUM_25);   // Libera el GPIO
      analogWrite(led_r, 0); //La lógica de los leds está invertida (Esto significa que para encender el led, el valor debe ser 0)
    }
    else{
      gpio_hold_dis(GPIO_NUM_25);  
      Serial.println("Battery ok");
      lowBattery=false;
      analogWrite(led_r, 255); //La lógica de los leds está invertida (Esto significa que para apagar el led, el valor debe ser 255)
    }
}

void loop() {

    /* ------------------------------------------------ Estado: Fncionando con la batería ------------------------------------------------ */
    if(chargingState == 0){
    
      /* ---------------------------------------------------------- MEDIDA SENSOR ---------------------------------------------------------- */
      sensors_event_t accel, gyro, temp; // Instancia de la estructura de eventos para almacenar las lecturas del sensor
      mpu.getEvent(&accel, &gyro, &temp);

      //Convertir las lecturas
      float ax = accel.acceleration.x;
      float ay = accel.acceleration.y;
      float az = accel.acceleration.z;

      float gx = gyro.gyro.x;
      float gy = gyro.gyro.y;
      float gz = gyro.gyro.z;

      float A2 = sqrt(ax*ax + ay*ay + az*az);// Magnitud del vector de aceleración
      float G2 = sqrt(gx*gx + gy*gy + gz*gz);// Magnitud del vector de velocidad angular
      
    
      //Serial.println(G2);
      
      /* ---------- DETECCIÓN MOVIMIENTO ---------- */
      if (G2 > 0.09){
          if (!moving) {
              moving = true;
              Serial.println("Movimiento detectado.");
          }
      } else {
          if (moving) {
              int side = determineCubeSide(ax, ay, az);
              if (side !=-1){ 

                moving = false;
                Serial.println("Movimiento detenido.");

                int lastSide = preferences.getInt("Side", 0);
                Serial.print("Side: ");
                Serial.print(side);
                Serial.print(" Last Side: ");
                Serial.println(lastSide);

                preferences.putInt("Side", side);

                if (side != lastSide){

                  sendHMI();

                  if(connectWiFi()){

                    if(modoDemo){

                      String url = "http://85.31.236.104:3047/send-emotion";  // Cambia la URL a la de tu servidor

                      // Especificar la URL del servidor
                      http.begin(url);  

                      // Configurar el tipo de contenido a enviar
                      http.addHeader("Content-Type", "application/json");

                      // Crear el cuerpo de la solicitud en formato JSON
                      String postData = "{\"emotion\":\"" + String(side) + "\"}";

                      // Enviar la solicitud POST
                      int httpResponseCode = http.POST(postData);  

                      // Comprobar el código de respuesta
                      if (httpResponseCode > 0) {
                        String response = http.getString();  // Obtener la respuesta del servidor
                        Serial.println("POST realizado con éxito, código de respuesta: " + String(httpResponseCode));
                        Serial.println("Respuesta: " + response);
                      } else {
                        Serial.println("Error en la solicitud POST, código de respuesta: " + String(httpResponseCode));
                      }

                      http.end();  // Finalizar la conexión
                      
                    }
                    else{
                      /* ---------- ENVÍO DE DATOS A SERVIDOR ----------*/
                      String macAddress = WiFi.macAddress();

          
                      String url = "https://www.unioviedo.es/medialab/datos_cube.php";

                      // Construir la URL con los parámetros en la cadena de consulta
                      url += "?e=" + String(side) + "&m=%27" + macAddress + "%27&b=" + String(battery);

                      Serial.println(url);
                      boolean success = false;
                      while(!success){
                        http.setTimeout(30000); // Tiempo de espera en milisegundos

                        http.begin(url);  // Iniciar la conexión

                        http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36");  // O usa un User-Agent específico de un navegador

                        // Enviar la solicitud GET
                        int httpResponseCode = http.GET();  // Realizar la solicitud GET

                        // Comprobar el código de respuesta
                        if (httpResponseCode == 200) {
                            Serial.println("GET realizado con éxito, código de respuesta: " + String(httpResponseCode));
                            success = true;
                        } else {
                            Serial.println("Error en la solicitud GET, código de respuesta: " + String(httpResponseCode));
                        }

                        http.end();  // Finalizar la conexión
                      }
                      
                      /* ---------- OBTENCIÓN HORA PARA OTA ---------- */
                      struct tm timeinfo;

                      configTime(0, 0, "hora.roa.es");
                      if (!getLocalTime(&timeinfo))
                      {
                        Serial.println("Failed to obtain time");
                                        }
                      Serial.println("Hora configurada de hora.roa.es");

                      String timezone = "CET-1CEST,M3.5.0/1,M10.5.0";
                      setenv("TZ", timezone.c_str(), 1);
                      tzset();

                      int currentHour = timeinfo.tm_hour;
                      int currentMinute = timeinfo.tm_min;

                      // Imprimir la hora actual
                      Serial.printf("Hora actual: %02d:%02d\n", currentHour, currentMinute);

                      // Verificar si la hora está entre las 00:00 y las 10:00
                      if (currentHour >= 2 && currentHour < 7) {
                          Serial.println("La hora está entre las 00:00 y las 10:00. Iniciando actualización OTA...");
                          //updateFirmware();  // Llamar a la función de actualización
                      } else {
                          Serial.println("No es la hora adecuada para la actualización.");
                      }
                    }
                  }
                }

                goToSleep();// Modo DEEP_SLEEP, comienza en el setup en la próxima interrupción (IMU o Carga)

              }
          }
      }
      

      delay(10);  // Espera 10ms antes de la siguiente lectura
    }

    /* ------------------------------------------------ Estado: Cargando & Configuración ------------------------------------------------ */
    else if(chargingState==1){
      int vchargeValue = analogRead(CHARGE_PIN);

      if(configFirst){
        createServer();
        configFirst = false;
      }

      if ((millis() - startTime < 300000) && vchargeValue > 1000) {
          configHMI();
          
          server.handleClient();
        } else {

          WiFi.softAPdisconnect(true);  // Desconectar AP
          chargingState=2;
          
        }
    }

    /* ------------------------------------------------ Estado: Después de carga o configuración ------------------------------------------------ */
    else{
      
      chargingHMI();
      int vchargeValue = analogRead(CHARGE_PIN);
      if (vchargeValue <1000){  
      
        battery = getBattery();

        ledsOff();
        delay(500);

        if (battery>75){
          for (int i = 0; i<4; i++){
            analogWrite(led_b, 255);
            analogWrite(led_g, 0);
            analogWrite(led_r, 255);
            delayLab(500);
            analogWrite(led_g, 255);
            analogWrite(led_b, 255);
            analogWrite(led_r, 255);
            delayLab(500);
          }
          
        }
        else if (battery>50)
        {
          for (int i = 0; i<3; i++){
            analogWrite(led_b, 255);
            analogWrite(led_g, 0);
            analogWrite(led_r, 255);
            delayLab(500);
            analogWrite(led_g, 255);
            analogWrite(led_b, 255);
            analogWrite(led_r, 255);
            delayLab(500);
          }
        }
        else if (battery>25)
        {
          for (int i = 0; i<2; i++){
            analogWrite(led_b, 255);
            analogWrite(led_g, 0);
            analogWrite(led_r, 255);
            delayLab(500);
            analogWrite(led_g, 255);
            analogWrite(led_b, 255);
            analogWrite(led_r, 255);
            delayLab(500);
          }
        }
        else{
            analogWrite(led_b, 255);
            analogWrite(led_g, 255);
            analogWrite(led_r, 0);
            delayLab(500);
            analogWrite(led_g, 255);
            analogWrite(led_b, 255);
            analogWrite(led_r, 255);
            delayLab(500);

          if (battery<15){
            lowBattery=true;
            analogWrite(led_r, 0);
          }
        }

        goToSleep();  
      } 
      
    }
  }
