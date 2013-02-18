/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __WHEELSGROUP_H
#define __WHEELSGROUP_H

#include <actuators.h>
#include "TracksController.h"


class WheelsGroup : public actuators::ActuatorGroup
{
 public:
  WheelsGroup(admin::StatePrx &prx, TracksController *tc);

 public:
  virtual admin::StatePrx getStateInterface(const Ice::Current& = Ice::Current());

  virtual actuators::ActuatorDescriptionSeq getActuatorDescription(const Ice::Current& = Ice::Current());

  virtual void setActuatorsAndWait(const actuators::ActuatorFrame &duties, 
				   const Ice::Current& = Ice::Current());

  virtual void setActuatorsNoWait(const actuators::ActuatorFrame &duties, 
				  const Ice::Current& = Ice::Current());

 private:
  admin::StatePrx state_prx;
  TracksController *tracks_ctl;
};

#endif // __WHEELSGROUP_H
