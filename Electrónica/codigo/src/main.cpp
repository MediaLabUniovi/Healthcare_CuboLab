#include <Arduino.h>
#include <esp_sleep.h>
#include "configuration.h"
#include "dfPlayerLab.h"
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
bool manualConfigRequested = false;

enum BatteryRuntimeState {
  BATTERY_RUNTIME_MONITORING = 0,
  BATTERY_RUNTIME_WAITING_DFPLAYER_OFF = 1,
};

enum DockBridgeState {
  DOCK_BRIDGE_WAIT_CHARGE = 0,
  DOCK_BRIDGE_WAIT_SERVER_DFPLAYER = 1,
  DOCK_BRIDGE_ACTIVE = 2,
};

BatteryRuntimeState batteryRuntimeState = BATTERY_RUNTIME_MONITORING;
unsigned long dfPlayerWaitStartMs = 0;
unsigned long lastDockChargingStatusMs = 0;
const unsigned long DOCK_CHARGING_STATUS_INTERVAL_MS = 500;
DockBridgeState dockBridgeState = DOCK_BRIDGE_WAIT_CHARGE;

namespace {
bool dockBridgeCanTx() {
  return dockBridgeState == DOCK_BRIDGE_ACTIVE;
}

void dockBridgeUpdateByCharge(bool chargingDetected) {
  if (!chargingDetected) {
    dockBridgeState = DOCK_BRIDGE_WAIT_CHARGE;
    return;
  }

  if (dockBridgeState == DOCK_BRIDGE_WAIT_CHARGE) {
    dockBridgeState = DOCK_BRIDGE_ACTIVE;
  }
}

void dockBridgeSendStatusIfAllowed(const char* phase, bool chargingDetected) {
  if (!chargingDetected || !dockBridgeCanTx()) {
    return;
  }
  dockCommSendStatus(phase);
}

void dockBridgeSendEventIfAllowed(const char* eventName, int value, bool chargingDetected) {
  if (!chargingDetected || !dockBridgeCanTx()) {
    return;
  }
  dockCommSendEvent(eventName, value);
}

void dockBridgeMaintainIfAllowed(bool chargingDetected) {
  if (!chargingDetected || !dockBridgeCanTx()) {
    return;
  }
  dockCommMaintainLink();
}
}  // namespace

void setup() {
  Serial.begin(115200);

  #if ENABLE_DFPLAYER
  pinMode(DFPLAYER_ENABLE_PIN, OUTPUT);
  digitalWrite(DFPLAYER_ENABLE_PIN, LOW);
  #endif

  pinMode(MPU_INT_PIN, INPUT_PULLUP);
  pinMode(CHARGE_PIN, INPUT);
  pinMode(CONFIG_BUTTON_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);

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

  esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();
  if (wakeCause == ESP_SLEEP_WAKEUP_EXT1) {
    uint64_t wakeMask = esp_sleep_get_ext1_wakeup_status();
    if ((wakeMask & (1ULL << MPU_INT_PIN)) != 0) {
      Serial.println("Wake por MPU INT: activando convertidor DFPlayer para calentamiento.");
      dfPlayerPowerOn();
    }
  }

  // Recuperar última cara conocida en NVS (actualizada por motionLab).
  side = preferences.getInt("Side", 0);
  Serial.printf("[SIDE] Última cara guardada: %d\n", side);

  battery = getBattery();
  Serial.println(battery);

  lowBattery = battery < 15;
  if (!lowBattery) {
    Serial.println("Battery ok");
    initDfPlayer();
  }

  bool chargingDetected = dockCommIsChargingDetected();
  bool configButtonDetected = isConfigButtonPressed();
  dockCommSetState(chargingDetected, battery, lowBattery, side);
  dockBridgeUpdateByCharge(chargingDetected);

  if (chargingDetected) {
    // Carga detectada: no implica automaticamente que la UART con base esté conectada.
    Serial.println("Dispositivo despertado por detección de carga en GPIO35.");
    chargingState = 1;
    startTime = millis();
  }

  if (configButtonDetected) {
    Serial.println("Botón de configuración detectado en GPIO33.");
    manualConfigRequested = true;
    chargingState = 1;
    startTime = millis();
  }
  dockBridgeSendStatusIfAllowed("BOOT", chargingDetected);
}

void loop() {
  processDfPlayerTimers();

  bool previousCharging = dockCommIsChargingDetected();
  bool chargingDetected = dockCommUpdateDebouncedChargeState();
  dockBridgeUpdateByCharge(chargingDetected);
  bool configButtonPressed = isConfigButtonPressed();
  dockCommSetState(chargingDetected, battery, lowBattery, side);

  if (configButtonPressed && !manualConfigRequested) {
    Serial.println("[CONFIG] Solicitud manual por botón GPIO33.");
    manualConfigRequested = true;
  }

  if (chargingDetected != previousCharging) {
    if (chargingDetected) {
      Serial.println("[CHARGE] Estado estable: cargando.");
      chargingState = 1;
      startTime = millis();
      side = preferences.getInt("Side", side);
      dockCommSetState(chargingDetected, battery, lowBattery, side);
      dockBridgeState = DOCK_BRIDGE_ACTIVE;
      dockBridgeSendEventIfAllowed("CHARGE_ON", 1, chargingDetected);
      dockBridgeSendStatusIfAllowed("CHARGING", chargingDetected);
      lastDockChargingStatusMs = millis();
    } else {
      Serial.println("[CHARGE] Estado estable: sin carga.");
      if (!manualConfigRequested) {
        chargingState = 2;
      }
      dockBridgeState = DOCK_BRIDGE_WAIT_CHARGE;
      dockBridgeSendEventIfAllowed("CHARGE_OFF", 0, chargingDetected);
      dockBridgeSendStatusIfAllowed("UNDOCKED", chargingDetected);
    }
  }

  dockBridgeMaintainIfAllowed(chargingDetected);

  if (chargingState == 0 && manualConfigRequested) {
    chargingState = 1;
    startTime = millis();
    dockBridgeSendEventIfAllowed("CONFIG_BTN", 1, chargingDetected);
    dockBridgeSendStatusIfAllowed("CONFIG_BUTTON", chargingDetected);
  }

  /* ------------------------------------------------ Estado: Fncionando con la batería ------------------------------------------------ */
  if (chargingState == 0) {
      if (batteryRuntimeState == BATTERY_RUNTIME_WAITING_DFPLAYER_OFF) {
        if (!isDfPlayerPowerOffPending()) {
          batteryRuntimeState = BATTERY_RUNTIME_MONITORING;
          goToSleep();
          return;
        }

        if ((millis() - dfPlayerWaitStartMs) >= DFPLAYER_SLEEP_WAIT_MS) {
          Serial.println("Timeout esperando apagado DFPlayer. Forzando apagado y sleep.");
          dfPlayerPowerOff();
          batteryRuntimeState = BATTERY_RUNTIME_MONITORING;
          goToSleep();
          return;
        }

        delay(10);
        return;
      }

      int detectedSide = -1;
      MotionCycleResult motionResult = processMotionCycle(mpu, preferences, moving, detectedSide);

      if (motionResult != MOTION_NO_EVENT) {
        if (motionResult == MOTION_SIDE_CHANGED) {
          sendHMI();
          side = detectedSide;
          dockCommSetState(chargingDetected, battery, lowBattery, side);
          dockBridgeState = DOCK_BRIDGE_WAIT_SERVER_DFPLAYER;

          bool telemetryOk = sendTelemetryForSide(http, modoDemo, detectedSide, battery);
          if (telemetryOk && isDfPlayerPowerOffPending()) {
            Serial.println("[DOCK] Telemetría OK y DFPlayer activo. Se habilita enlace con base.");
          } else if (!telemetryOk) {
            Serial.println("[DOCK] Telemetría fallida. Se reanuda política UART por estado de carga.");
          } else {
            Serial.println("[DOCK] Telemetría OK pero DFPlayer no pendiente. Se reanuda política UART por estado de carga.");
          }

          dockBridgeState = chargingDetected ? DOCK_BRIDGE_ACTIVE : DOCK_BRIDGE_WAIT_CHARGE;
          dockBridgeSendEventIfAllowed("SIDE_CHANGED", detectedSide, chargingDetected);
          dockBridgeSendStatusIfAllowed("MOTION_DONE", chargingDetected);
        }

        // Si hay audio pendiente, esperamos su autoapagado antes de dormir.
        if (isDfPlayerPowerOffPending()) {
          batteryRuntimeState = BATTERY_RUNTIME_WAITING_DFPLAYER_OFF;
          dfPlayerWaitStartMs = millis();
          Serial.println("Esperando fin de ventana DFPlayer antes de dormir.");
        } else {
          goToSleep();
        }
      }


      delay(10);  // Espera 10ms antes de la siguiente lectura
  }

  /* ------------------------------------------------ Estado: Cargando & Configuración ------------------------------------------------ */
  else if (chargingState == 1) {
    battery = getBattery();
    lowBattery = battery < 15;

    // Durante carga se reporta estado a la base para que la OLED muestre información.
    dockCommSetState(chargingDetected, battery, lowBattery, side);
    if (dockBridgeCanTx() &&
        dockCommIsConnected() &&
        (millis() - lastDockChargingStatusMs) >= DOCK_CHARGING_STATUS_INTERVAL_MS) {
      dockBridgeSendStatusIfAllowed("CHARGING", chargingDetected);
      lastDockChargingStatusMs = millis();
    }

    if (configFirst) {
      createServer();
      configFirst = false;
    }

    if ((millis() - startTime < 300000) && (chargingDetected || manualConfigRequested)) {
      configHMI();
      server.handleClient();
    } else {

      WiFi.softAPdisconnect(true);  // Desconectar AP
      chargingState = 2;
      manualConfigRequested = false;
    }
  }

  /* ------------------------------------------------ Estado: Después de carga o configuración ------------------------------------------------ */
  else {
    // Hardware nuevo sin LEDs locales: se elimina secuencia visual post-carga.
    // Se deja una ventana corta para que la base pinte el estado final y luego dormimos.
    battery = getBattery();
    lowBattery = battery < 15;
    dockCommSetState(chargingDetected, battery, lowBattery, side);
    dockBridgeSendStatusIfAllowed("POST_CHARGE", chargingDetected);
    delay(500);
    goToSleep();
  }
}
