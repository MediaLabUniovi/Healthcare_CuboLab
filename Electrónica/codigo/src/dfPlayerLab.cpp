#include "dfPlayerLab.h"

#include "configuration.h"

#if ENABLE_DFPLAYER
#include <DFRobotDFPlayerMini.h>
#endif

namespace {
#if ENABLE_DFPLAYER
HardwareSerial dfPlayerSerial(1);
DFRobotDFPlayerMini dfPlayer;

bool dfPlayerPowered = false;
bool dfPlayerReady = false;
bool powerOffScheduled = false;
unsigned long powerOffDeadlineMs = 0;
unsigned long powerOnTimestampMs = 0;

// true  -> usa /mp3/0001.mp3 mediante playMp3Folder()
// false -> usa raíz 0001.mp3 mediante play()
constexpr bool kUseMp3FolderLayout = false;

void waitWithTimer(unsigned long waitMs) {
  unsigned long start = millis();
  while ((millis() - start) < waitMs) {
    yield();
  }
}

void flushDfPlayerRx() {
  while (dfPlayerSerial.available() > 0) {
    (void)dfPlayerSerial.read();
  }
}

bool tryInitWithPins(int rxPin, int txPin, const char* label) {
  dfPlayerSerial.end();
  dfPlayerSerial.begin(9600, SERIAL_8N1, rxPin, txPin);
  waitWithTimer(80);
  flushDfPlayerRx();

  for (int attempt = 1; attempt <= DFPLAYER_INIT_RETRIES; attempt++) {
    if (dfPlayer.begin(dfPlayerSerial, true, true)) {
      // Verifica canal TX real enviando una consulta de estado.
      int volumeRead = dfPlayer.readVolume();
      if (volumeRead >= 0) {
        dfPlayerReady = true;
        dfPlayer.volume(22);
        Serial.printf("DFPlayer inicializado correctamente (%s, RX=%d TX=%d).\n", label, rxPin, txPin);
        return true;
      }

      Serial.printf("DFPlayer sin respuesta a comandos (%s, intento %d/%d).\n", label, attempt, DFPLAYER_INIT_RETRIES);
    } else {
      Serial.printf("Fallo al inicializar DFPlayer (%s, intento %d/%d).\n", label, attempt, DFPLAYER_INIT_RETRIES);
    }

    waitWithTimer(200);
    flushDfPlayerRx();
  }

  return false;
}
#endif
}  // namespace

void dfPlayerPowerOn() {
#if ENABLE_DFPLAYER
  if (dfPlayerPowered) {
    return;
  }

  pinMode(DFPLAYER_ENABLE_PIN, OUTPUT);
  digitalWrite(DFPLAYER_ENABLE_PIN, HIGH);
  dfPlayerPowered = true;
  powerOffScheduled = false;
  powerOnTimestampMs = millis();
#endif
}

void dfPlayerPowerOff() {
#if ENABLE_DFPLAYER
  if (!dfPlayerPowered) {
    return;
  }

  powerOffScheduled = false;
  dfPlayerReady = false;
  dfPlayerSerial.end();
  digitalWrite(DFPLAYER_ENABLE_PIN, LOW);
  dfPlayerPowered = false;
#endif
}

bool initDfPlayer() {
#if ENABLE_DFPLAYER
  if (dfPlayerReady) {
    return true;
  }

  dfPlayerPowerOn();

  if ((millis() - powerOnTimestampMs) < DFPLAYER_POWER_STABILIZE_MS) {
    waitWithTimer(DFPLAYER_POWER_STABILIZE_MS - (millis() - powerOnTimestampMs));
  }

  // Mapa principal según comentarios de configuration.h
  if (tryInitWithPins(DFPLAYER_UART_TX_PIN, DFPLAYER_UART_RX_PIN, "mapa_principal")) {
    return true;
  }

  // Fallback para descartar inversión RX/TX en cableado o macros.
  if (tryInitWithPins(DFPLAYER_UART_RX_PIN, DFPLAYER_UART_TX_PIN, "mapa_fallback")) {
    return true;
  }

  Serial.println("DFPlayer no disponible. Se continúa sin audio.");
  dfPlayerReady = false;
  return false;
#else
  return false;
#endif
}

void playEventTrack(uint16_t trackId) {
#if ENABLE_DFPLAYER
  if (!dfPlayerReady && !initDfPlayer()) {
    return;
  }

  if (kUseMp3FolderLayout) {
    Serial.printf("DFPlayer PLAY track=%u (layout: /mp3/000N.mp3).\n", trackId);
    dfPlayer.playMp3Folder(trackId);
  } else {
    Serial.printf("DFPlayer PLAY track=%u (layout: raíz 000N.mp3).\n", trackId);
    dfPlayer.play(trackId);
  }

  // readState() puede devolver -1 incluso reproduciendo; se deja solo como traza.
  waitWithTimer(120);
  int state = dfPlayer.readState();
  Serial.printf("DFPlayer state tras PLAY=%d\n", state);

  scheduleDfPlayerPowerOff(DFPLAYER_AUTO_POWER_OFF_MS);
#else
  (void)trackId;
#endif
}

void scheduleDfPlayerPowerOff(uint32_t delayMs) {
#if ENABLE_DFPLAYER
  if (!dfPlayerPowered) {
    return;
  }

  powerOffDeadlineMs = millis() + delayMs;
  powerOffScheduled = true;
#else
  (void)delayMs;
#endif
}

void processDfPlayerTimers() {
#if ENABLE_DFPLAYER
  if (!powerOffScheduled) {
    return;
  }

  if ((long)(millis() - powerOffDeadlineMs) >= 0) {
    dfPlayerPowerOff();
  }
#endif
}

void waitDfPlayerTimers(uint32_t maxWaitMs) {
#if ENABLE_DFPLAYER
  unsigned long start = millis();
  while (powerOffScheduled && (millis() - start) < maxWaitMs) {
    processDfPlayerTimers();
    yield();
  }
#else
  (void)maxWaitMs;
#endif
}

bool isDfPlayerPowerOffPending() {
#if ENABLE_DFPLAYER
  return powerOffScheduled;
#else
  return false;
#endif
}
