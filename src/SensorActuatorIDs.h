/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __SENSORACTUATORIDS_H
#define __SENSORACTUATORIDS_H

typedef enum 
{
  CAMERA0 = 0,
  CAMERA1,
  SONAR,
  COMPASS,
  GPS
} SensorID;

typedef enum 
{
  MOTOR0 = 0,
  MOTOR1,
  CAMERA_SERVO
} ActuatorID;

#endif // __SENSORACTUATORIDS_H
