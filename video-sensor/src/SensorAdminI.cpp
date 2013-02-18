/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "SensorAdminI.h"


SensorAdminI::SensorAdminI()
  : sensor_group(0)
{
}


void 
SensorAdminI::setSensorGroup(SensorGroupIPtr &sg)
{
  this->sensor_group = sg;
}


void
SensorAdminI::start(const Ice::Current&)
{
  if(this->sensor_group)
    this->sensor_group->requestStart();
}


void
SensorAdminI::stop(const Ice::Current&)
{
  if(this->sensor_group)
    this->sensor_group->requestStop();
}
