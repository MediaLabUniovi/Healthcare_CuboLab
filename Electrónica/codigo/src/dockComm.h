#ifndef DOCKCOMM_H
#define DOCKCOMM_H

#include <Arduino.h>
#include "configuration.h"

void dockCommBegin(HardwareSerial& serialPort);
void dockCommSetState(bool charging, int batteryLevel, bool lowBatteryState);

bool dockCommIsChargingDetected();
bool dockCommIsConnected();

bool dockCommUpdateDebouncedChargeState();
bool dockCommAttemptHandshake();
void dockCommMaintainLink();

void dockCommSendStatus(const char* phase);
void dockCommSendEvent(const char* eventName, int value);

#endif
