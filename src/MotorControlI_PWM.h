/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __MOTORCONTROLI_H
#define __MOTORCONTROLI_H

#include <IceE/Properties.h>
#include <vehicle.h>
#ifdef WIN32
#include "msinttypes/inttypes.h"
#else
#include <inttypes.h>
#endif

class MotorControlI
{
 public:
  MotorControlI(const Ice::PropertiesPtr &properties);
  ~MotorControlI();

 public:
  virtual void setDuties(const vehicle::ActuatorFrame &duties);

 private:
  int mem_fd;
  size_t motor_left_gpio; 
  size_t motor_right_gpio; 
  float motor_left_range_lo;
  float motor_left_range_hi;
  float motor_right_range_lo;
  float motor_right_range_hi;

  uint8_t *gpt_left;
  uint8_t *gpt_right;
  uint32_t resolution;

  float prev_left;
  float prev_right;
  short prev_steer;
  short prev_accel;
};

#endif // __MOTORCONTROLI_H
