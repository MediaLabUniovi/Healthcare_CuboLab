#ifndef TELEMETRYLAB_H
#define TELEMETRYLAB_H

#include "configuration.h"

bool sendTelemetryForSide(HTTPClient& http, bool modoDemo, int side, int battery);

#endif
