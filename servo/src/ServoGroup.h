/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __SERVOGROUP_H
#define __SERVOGROUP_H

#include <actuators.h>
#include <inttypes.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Handle.h>
#include "dm3730-pwm.h"


class ServoGroup : public actuators::ActuatorGroup
{
 public:
  ServoGroup(admin::StatePrx &prx);

  void setStarted(bool started);

 public:
  virtual admin::StatePrx getStateInterface(const Ice::Current& = Ice::Current());

  virtual actuators::ActuatorDescriptionSeq getActuatorDescription(const Ice::Current& = Ice::Current());

  virtual void setActuatorsAndWait(const actuators::ActuatorFrame &duties, 
				   const Ice::Current& = Ice::Current());

  virtual void setActuatorsNoWait(const actuators::ActuatorFrame &duties, 
				  const Ice::Current& = Ice::Current());

 private:
  admin::StatePrx state_prx;
  bool is_started;
  int mem_fd;
  uint8_t *gpio_timer;
  uint32_t resolution;
  IceUtil::Mutex hw_mutex;
  float current_duty;
};

typedef IceUtil::Handle<ServoGroup> ServoGroupPtr;

#endif // __SERVOGROUP_H
