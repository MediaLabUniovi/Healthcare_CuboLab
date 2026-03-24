#ifndef MOTIONLAB_H
#define MOTIONLAB_H

#include "configuration.h"

// Resultado de la evaluación de movimiento del cubo en un ciclo de loop.
enum MotionCycleResult {
  MOTION_NO_EVENT,
  MOTION_SIDE_DETERMINED,
  MOTION_SIDE_CHANGED
};

MotionCycleResult processMotionCycle(Adafruit_MPU6050& mpu,
                                     Preferences& preferences,
                                     bool& moving,
                                     int& detectedSide);

#endif
