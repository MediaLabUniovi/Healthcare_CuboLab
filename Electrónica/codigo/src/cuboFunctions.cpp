#include "cuboFunctions.h"

namespace {
bool dfPlayerReady = false;
constexpr unsigned long buzzerDurationMs = 180;

// DFPlayer UART protocol: start, version, length, command, feedback,
// parameter MSB/LSB, checksum MSB/LSB, end.
void sendAudioCommand(uint8_t command, uint16_t parameter) {
  uint8_t frame[10] = {0x7e, 0xff, 0x06, command, 0x00,
                       uint8_t(parameter >> 8), uint8_t(parameter), 0, 0, 0xef};
  uint16_t checksum = 0;
  for (int i = 1; i <= 6; ++i) checksum -= frame[i];
  frame[7] = checksum >> 8;
  frame[8] = checksum;
  Serial2.write(frame, sizeof(frame));
  Serial2.flush();
}

bool waitForVolumeResponse() {
  uint8_t frame[10];
  size_t length = 0;
  unsigned long started = millis();
  while (millis() - started < 1500) {
    if (!Serial2.available()) {
      delay(1);
      continue;
    }
    uint8_t value = Serial2.read();
    if (length == 0 && value != 0x7e) continue;
    frame[length++] = value;
    if (length != sizeof(frame)) continue;
    length = 0;
    uint16_t checksum = 0;
    for (int i = 1; i <= 6; ++i) checksum -= frame[i];
    if (frame[1] == 0xff && frame[2] == 0x06 && frame[3] == 0x43 &&
        frame[5] == 0 && frame[6] <= 30 && frame[9] == 0xef &&
        frame[7] == uint8_t(checksum >> 8) && frame[8] == uint8_t(checksum)) {
      return true;
    }
  }
  return false;
}

void playAudioEvent(uint16_t track) {
  if (dfPlayerReady) {
    sendAudioCommand(0x12, track); // /mp3/0001.mp3, /mp3/0002.mp3
  } else {
    tone(BUZZER_PIN, 880, buzzerDurationMs);
    // Complete the beep before callers can enter deep sleep.
    delay(buzzerDurationMs);
    noTone(BUZZER_PIN);
  }
}
}

void initAudio() {
  dfPlayerReady = false; // Probe again at every boot, including deep-sleep wakeup.
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial2.begin(9600, SERIAL_8N1, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
  delay(1000); // Allow the module and SD card to start.
  while (Serial2.available()) Serial2.read();
  sendAudioCommand(0x43, 0); // Query volume; require a valid response.
  dfPlayerReady = waitForVolumeResponse();
  if (dfPlayerReady) {
    sendAudioCommand(0x06, 20); // Volume: 0..30.
    delay(100);
    Serial.println("Audio: DFPlayer disponible.");
  } else {
    Serial.println("Audio: DFPlayer sin respuesta; buzzer en GPIO4.");
  }
}

boolean networksAvailable = false;
String networks = "";

float tolerance = 1;  // Rango de tolerancia para comparar los valores de los ejes

float ax_values[6];  // Para almacenar ax de cada lado
float ay_values[6];  // Para almacenar ay de cada lado
float az_values[6];  // Para almacenar az de cada lado

int minBatt = 2630; // Valor mínimo de la batería con divisor de 0.35: 5,95V
int maxBatt = 3680; // Valor mínimo de la batería con divisor de 0.35: 8,2V

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

    analogWrite(led_r, 0);
    playAudioEvent(2);
    delayLab(300);
    analogWrite(led_r, 255);
    delayLab(300);

    analogWrite(led_r, 0);
    delayLab(500);
    analogWrite(led_r, 255);
    delayLab(500);

    analogWrite(led_r, 0);
    delayLab(1000);
    analogWrite(led_r, 255);
    delayLab(500);


    return false;
  }

  return true;
}

void ledsOff(){
  // La lógica de los leds está invertida
    analogWrite(led_r, 255);
    analogWrite(led_g, 255);
    analogWrite(led_b, 255);
}

void delayLab(long wait){
  // Funcion para reemplazar delay
  long tt =millis();
  while((millis()-tt)<wait){}
}

void sendHMI(){
  // Función para indicar que se ha detectado un movimiento
    ledsOff();
    analogWrite(led_g, 0);
    playAudioEvent(1);
}


void configHMI(){
  // Función para indicar que se está configurando el cubo
    ledsOff();
    analogWrite(led_b, 255);
    analogWrite(led_g, 255);
    analogWrite(led_r, 0);
    delayLab(500);
    analogWrite(led_g, 0);
    analogWrite(led_b, 255);
    analogWrite(led_r, 255);
    delayLab(500);
}

void chargingHMI(){
  // Función para indicar que se está cargando el cubo
    if (getBattery()==100){
      analogWrite(led_g, 0);
    } else{
    ledsOff();
    for (int brillo = 0; brillo <= 255; brillo++) {
    analogWrite(led_g, brillo);
    delayLab(10);
    }

    for (int brillo = 255; brillo >= 0; brillo--) {
    analogWrite(led_g, brillo);
    delayLab(10);
    }
    ledsOff();
  }
}

void testLowBattery(){
  // Función para probar el estado de la batería
  if(lowBattery){
    analogWrite(led_r, 0);
  }
  else{
    analogWrite(led_r, 255);
  }
}

int getBattery(){
  // Leer el valor de la batería
  int batt = analogRead(34);
  Serial.print("Battery Read: ");
  Serial.println(batt);
  if (batt<minBatt){batt = 0;}
  else{
     batt = batt-minBatt;
  }
  batt = batt * 100 / (maxBatt-minBatt);
  if (batt>100){batt = 100;}
  return batt;
}

void goToSleep(){
    // Función para poner el dispositivo en modo de bajo consumo
    if(lowBattery){
      Serial.println("Low Battery");
      analogWrite(led_r, 0);
      delay(500);
      gpio_hold_en(GPIO_NUM_25);  // GPIO de led_r
      gpio_deep_sleep_hold_en();  // Habilitar GPIO hold para deep sleep

    }

    // Entrar en modo deep sleep para ahorrar batería
    Serial.println("Entrando en modo de bajo consumo.");

    // Habilitar desperetar por interrupción en movimiento e interrupción de carga
    esp_sleep_enable_ext1_wakeup((1ULL << MPU_INT_PIN) | (1ULL << CHARGE_PIN), ESP_EXT1_WAKEUP_ANY_HIGH); 
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
