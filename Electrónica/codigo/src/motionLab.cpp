#include "motionLab.h"

MotionCycleResult processMotionCycle(Adafruit_MPU6050& mpu,
                                     Preferences& preferences,
                                     bool& moving,
                                     int& detectedSide) {
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  float ax = accel.acceleration.x;
  float ay = accel.acceleration.y;
  float az = accel.acceleration.z;

  float gx = gyro.gyro.x;
  float gy = gyro.gyro.y;
  float gz = gyro.gyro.z;

  float g2 = sqrt(gx * gx + gy * gy + gz * gz);

  // Si hay rotación, seguimos en estado "moving" y no disparamos eventos.
  if (g2 > 0.09f) {
    if (!moving) {
      moving = true;
      Serial.println("Movimiento detectado.");
    }
    return MOTION_NO_EVENT;
  }

  if (!moving) {
    return MOTION_NO_EVENT;
  }

  detectedSide = determineCubeSide(ax, ay, az);
  if (detectedSide == -1) {
    return MOTION_NO_EVENT;
  }

  moving = false;
  Serial.println("Movimiento detenido.");

  int lastSide = preferences.getInt("Side", 0);
  Serial.print("Side: ");
  Serial.print(detectedSide);
  Serial.print(" Last Side: ");
  Serial.println(lastSide);

  preferences.putInt("Side", detectedSide);

  if (detectedSide != lastSide) {
    return MOTION_SIDE_CHANGED;
  }

  return MOTION_SIDE_DETERMINED;
}
