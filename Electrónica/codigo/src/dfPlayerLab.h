#ifndef DFPLAYERLAB_H
#define DFPLAYERLAB_H

#include <Arduino.h>

void dfPlayerPowerOn();
void dfPlayerPowerOff();
bool initDfPlayer();
void playEventTrack(uint16_t trackId);

void scheduleDfPlayerPowerOff(uint32_t delayMs);
void processDfPlayerTimers();
void waitDfPlayerTimers(uint32_t maxWaitMs);
bool isDfPlayerPowerOffPending();

#endif  // DFPLAYERLAB_H
