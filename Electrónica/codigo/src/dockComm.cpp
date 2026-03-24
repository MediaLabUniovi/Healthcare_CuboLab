#include "dockComm.h"

namespace {
HardwareSerial* dockSerial = nullptr;

bool chargingDetected = false;
bool dockConnected = false;

bool chargeRawState = false;
bool chargeStableState = false;
unsigned long chargeLastEdgeMs = 0;

String dockRxBuffer;
unsigned long lastDockRxMs = 0;
unsigned long lastDockPingMs = 0;

int currentBattery = 0;
bool currentLowBattery = false;

void sendDockLine(const String& line) {
  if (dockSerial == nullptr) {
    return;
  }

  dockSerial->println(line);
  Serial.println("[DOCK TX] " + line);
}

void processDockLine(const String& line) {
  if (line.length() == 0) {
    return;
  }

  Serial.println("[DOCK RX] " + line);
  lastDockRxMs = millis();

  if (line == "HELLO_ACK") {
    dockConnected = true;
    return;
  }

  if (line == "PING") {
    sendDockLine("ACK");
    return;
  }

  if (line == "STATUS?") {
    dockCommSendStatus("QUERY");
    return;
  }

  if (line.startsWith("ACK")) {
    // Reservado para ampliaciones (ACK de comandos con secuencia).
    return;
  }
}

void processDockSerial() {
  if (dockSerial == nullptr) {
    return;
  }

  while (dockSerial->available() > 0) {
    char c = (char)dockSerial->read();
    if (c == '\n' || c == '\r') {
      if (dockRxBuffer.length() > 0) {
        processDockLine(dockRxBuffer);
        dockRxBuffer = "";
      }
    } else {
      dockRxBuffer += c;
      if (dockRxBuffer.length() > 120) {
        dockRxBuffer = "";
      }
    }
  }
}
}  // namespace

void dockCommBegin(HardwareSerial& serialPort) {
  dockSerial = &serialPort;

  // Inicializa el debounce con el estado real al arrancar para evitar falsos flancos.
  chargeRawState = isChargingActive();
  chargeStableState = chargeRawState;
  chargeLastEdgeMs = millis();
  chargingDetected = chargeStableState;
}

void dockCommSetState(bool charging, int batteryLevel, bool lowBatteryState) {
  chargingDetected = charging;
  currentBattery = batteryLevel;
  currentLowBattery = lowBatteryState;
}

bool dockCommIsChargingDetected() {
  return chargingDetected;
}

bool dockCommIsConnected() {
  return dockConnected;
}

bool dockCommUpdateDebouncedChargeState() {
  bool rawNow = isChargingActive();
  if (rawNow != chargeRawState) {
    chargeRawState = rawNow;
    chargeLastEdgeMs = millis();
  }

  if ((millis() - chargeLastEdgeMs) >= CHARGE_DEBOUNCE_MS) {
    chargeStableState = chargeRawState;
  }

  chargingDetected = chargeStableState;
  return chargingDetected;
}

bool dockCommAttemptHandshake() {
  for (int attempt = 1; attempt <= DOCK_HANDSHAKE_RETRIES; attempt++) {
    sendDockLine("HELLO");

    unsigned long deadline = millis() + DOCK_HELLO_TIMEOUT_MS;
    while ((long)(deadline - millis()) > 0) {
      processDockSerial();
      if (dockConnected) {
        dockCommSendStatus("HANDSHAKE_OK");
        return true;
      }
      delay(5);
    }

    Serial.printf("[DOCK] HELLO timeout (%d/%d).\n", attempt, DOCK_HANDSHAKE_RETRIES);
  }

  return false;
}

void dockCommMaintainLink() {
  processDockSerial();

  if (!dockConnected) {
    return;
  }

  if (millis() - lastDockPingMs >= DOCK_PING_INTERVAL_MS) {
    sendDockLine("PING");
    lastDockPingMs = millis();
  }

  // Si no llega nada de la base durante mucho tiempo, asumimos enlace caído.
  if (millis() - lastDockRxMs > (DOCK_PING_INTERVAL_MS + DOCK_ACK_TIMEOUT_MS)) {
    dockConnected = false;
    Serial.println("[DOCK] Enlace perdido.");
  }
}

void dockCommSendStatus(const char* phase) {
  String msg = "STATUS;phase=" + String(phase) +
               ";charging=" + String(chargingDetected ? 1 : 0) +
               ";battery=" + String(currentBattery) +
               ";low=" + String(currentLowBattery ? 1 : 0) +
               ";dock=" + String(dockConnected ? 1 : 0);
  sendDockLine(msg);
}

void dockCommSendEvent(const char* eventName, int value) {
  String msg = "EVENT;name=" + String(eventName);
  if (value >= 0) {
    msg += ";value=" + String(value);
  }
  sendDockLine(msg);
}
