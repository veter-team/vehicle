#include "WheelsGroup.h"


WheelsGroup::WheelsGroup(admin::StatePrx &prx, TracksController *tc)
  : state_prx(prx),
    tracks_ctl(tc)
{
}


admin::StatePrx 
WheelsGroup::getStateInterface(const Ice::Current&)
{
  return this->state_prx;
}


actuators::ActuatorDescriptionSeq 
WheelsGroup::getActuatorDescription(const Ice::Current&)
{
  actuators::ActuatorDescriptionSeq ret;
  actuators::ActuatorDescription desc;

  desc.id = 0;
  desc.description = "Left track";
  desc.vendorid = "tb6612fng";
  ret.push_back(desc);
  desc.id = 1;
  desc.description = "Right track";
  desc.vendorid = "tb6612fng";
  ret.push_back(desc);

  return ret;
}


void 
WheelsGroup::setActuatorsAndWait(const actuators::ActuatorFrame &duties, 
				 const Ice::Current&)
{
  this->tracks_ctl->setActuators(duties, true);
}

void 
WheelsGroup::setActuatorsNoWait(const actuators::ActuatorFrame &duties, 
				 const Ice::Current&)
{
  this->tracks_ctl->setActuators(duties, false);
}
