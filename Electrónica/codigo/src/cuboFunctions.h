#ifndef CUBOFUNCTIONS_H
#define CUBOFUNCTIONS_H

#include "configuration.h"

extern boolean networksAvailable;
extern String networks;

extern float ax_values[6];  // Para almacenar ax de cada lado
extern float ay_values[6];  // Para almacenar ay de cada lado
extern float az_values[6];  // Para almacenar az de cada lado
extern int minBatt;
extern int maxBatt;

void getLimits();
int determineCubeSide(float ax, float ay, float az);
boolean connectWiFi();
void ledsOff();
void delayLab(long wait);
void sendHMI();
void configHMI();
void chargingHMI();
void testLowBattery();
void goToSleep();
void scanNetworks();
float mean(float readings[], int size);
int getBattery();

#endif // WIFILAB_H