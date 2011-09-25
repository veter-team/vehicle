/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __SERVOCONTROL_H
#define __SERVOCONTROL_H

#include <IceE/Properties.h>
#include <vehicle.h>
#ifdef WIN32
#include "msinttypes/inttypes.h"
#else
#include <inttypes.h>
#endif

class ServoControl
{
 public:
  ServoControl(const Ice::PropertiesPtr &properties,
               const std::string &servo_name);
  ~ServoControl();

 public:
  virtual void setDuties(const vehicle::ActuatorData &data);

};

#endif // __SERVOCONTROL_H
