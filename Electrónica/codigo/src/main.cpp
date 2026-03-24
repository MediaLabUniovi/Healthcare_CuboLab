#include <Arduino.h>
#include "configuration.h"
#include "dockComm.h"
#include "motionLab.h"
#include "telemetryLab.h"

Adafruit_MPU6050 mpu;
HardwareSerial auxUart(2);

const float accelThreshold = 0.05;  // Umbral para detectar movimiento en g
const float gyroThreshold = 0.1;    // Umbral para detectar rotación en rad/s

// Opciones de configuración del MPU6050
const float motion = 1;
const float motionDuration = 1.0;

boolean modoDemo = false;
int side = 0;  // Variable para rastrear el lado del cubo (de 0 a 5)

boolean calibration = true;

HTTPClient http;

int chargingState = 0;  // Estado de carga del dispositivo (0: no cargando, 1: cargando, 2: cargado)
WiFiMulti wifiMulti;  // Instancia de WiFiMulti para conectarse a varias redes WiFi
Preferences preferences;  // Instancia de Preferences para almacenar los valores de los ejes en memoria no volátil
unsigned long startTime;

int battery;
bool lowBattery = false;
bool moving = true;

bool configFirst = true;

void setup() {
  Serial.begin(115200);

  pinMode(DFPLAYER_ENABLE_PIN, OUTPUT);
  digitalWrite(DFPLAYER_ENABLE_PIN, LOW);

  pinMode(MPU_INT_PIN, INPUT_PULLUP);
  pinMode(CHARGE_PIN, INPUT);

  Wire.begin(MPU_SDA_PIN, MPU_SCL_PIN);

  auxUart.begin(AUX_UART_BAUDRATE, SERIAL_8N1, AUX_UART_RX_PIN, AUX_UART_TX_PIN);
  Serial.printf("UART auxiliar configurada. RX=%d TX=%d\n", AUX_UART_RX_PIN, AUX_UART_TX_PIN);
  dockCommBegin(auxUart);

  if (AUX_UART_TX_PIN == 15 || AUX_UART_TX_PIN == 5 || AUX_UART_TX_PIN == 12) {
    Serial.println("WARNING: El pin TX de la UART auxiliar usa un pin de arranque. Revisar cableado y niveles al boot.");
  }
  if (AUX_UART_RX_PIN == 15) {
    Serial.println("WARNING: GPIO15 es pin sensible de arranque. Evitar pull-ups/pull-downs fuertes desde la base.");
  }

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

  preferences.begin("MediaLab", false);

  getLimits();  // Recuperar los valores de los ejes x, y, z para cada lado del cubo

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  // Configurar el MPU6050 para activar la interrupción en movimiento
  mpu.setMotionInterrupt(true);
  mpu.setMotionDetectionThreshold(motion);
  mpu.setMotionDetectionDuration(motionDuration);

  bool chargingDetected = dockCommIsChargingDetected();

  if (chargingDetected) {
    // Carga detectada: no implica automaticamente que la UART con base esté conectada.
    Serial.println("Dispositivo despertado por detección de carga en GPIO35.");
    chargingState = 1;
    startTime = millis();
    dockCommAttemptHandshake();
  }

  battery = getBattery();
  Serial.println(battery);

  lowBattery = battery < 15;
  if (!lowBattery) {
    Serial.println("Battery ok");
    initDfPlayer();
  }

  dockCommSetState(chargingDetected, battery, lowBattery);
  dockCommSendStatus("BOOT");
}

void loop() {
  bool previousCharging = dockCommIsChargingDetected();
  bool chargingDetected = dockCommUpdateDebouncedChargeState();
  dockCommSetState(chargingDetected, battery, lowBattery);

  if (chargingDetected != previousCharging) {
    if (chargingDetected) {
      Serial.println("[CHARGE] Estado estable: cargando.");
      chargingState = 1;
      startTime = millis();
      if (!dockCommIsConnected()) {
        dockCommAttemptHandshake();
      }
      dockCommSendEvent("CHARGE_ON", 1);
      dockCommSendStatus("CHARGING");
    } else {
      Serial.println("[CHARGE] Estado estable: sin carga.");
      chargingState = 2;
      dockCommSendEvent("CHARGE_OFF", 0);
      dockCommSendStatus("UNDOCKED");
    }
  }

  dockCommMaintainLink();

  /* ------------------------------------------------ Estado: Fncionando con la batería ------------------------------------------------ */
  if (chargingState == 0) {
      int detectedSide = -1;
      MotionCycleResult motionResult = processMotionCycle(mpu, preferences, moving, detectedSide);

      if (motionResult != MOTION_NO_EVENT) {
        if (motionResult == MOTION_SIDE_CHANGED) {
          sendHMI();
          dockCommSetState(chargingDetected, battery, lowBattery);
          dockCommSendEvent("SIDE_CHANGED", detectedSide);
          dockCommSendStatus("MOTION_DONE");
          sendTelemetryForSide(http, modoDemo, detectedSide, battery);
        }

        // Tras estabilizar lado, mantenemos la política anterior: volver a deep sleep.
        goToSleep();
      }


      delay(10);  // Espera 10ms antes de la siguiente lectura
  }

  /* ------------------------------------------------ Estado: Cargando & Configuración ------------------------------------------------ */
  else if (chargingState == 1) {
    battery = getBattery();
    lowBattery = battery < 15;

    // Durante carga se reporta estado a la base para que la OLED muestre información.
    dockCommSetState(chargingDetected, battery, lowBattery);
    dockCommSendStatus("CHARGING");

    if (configFirst) {
      createServer();
      configFirst = false;
    }

    if ((millis() - startTime < 300000) && chargingDetected) {
      configHMI();
      server.handleClient();
    } else {

      WiFi.softAPdisconnect(true);  // Desconectar AP
      chargingState = 2;
    }
  }

  /* ------------------------------------------------ Estado: Después de carga o configuración ------------------------------------------------ */
  else {
    // Hardware nuevo sin LEDs locales: se elimina secuencia visual post-carga.
    // Se deja una ventana corta para que la base pinte el estado final y luego dormimos.
    battery = getBattery();
    lowBattery = battery < 15;
    dockCommSetState(chargingDetected, battery, lowBattery);
    dockCommSendStatus("POST_CHARGE");
    delay(500);
    goToSleep();
  }
}
