#ifndef SERVERLAB_H
#define SERVERLAB_H

#include "configuration.h"

void createServer();
void handleAddWifi();
void handleCalibrate();

// Declaration of global variables
extern WebServer server;  
extern String macStr;   
extern int calibrationSide;  

extern boolean wifiUpdate;  // Declare, but don't initialize it




#endif // WIFILAB_H