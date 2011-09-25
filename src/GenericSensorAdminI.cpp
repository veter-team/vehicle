/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "GenericSensorAdminI.h"


GenericSensorAdminI::GenericSensorAdminI(const Ice::PropertiesPtr &properties,
                                         NavSensorThreadPtr nst,
                                         SensorID sid)
: nav_sensor_thread(nst),
  sensor_id(sid)
{
}


GenericSensorAdminI::~GenericSensorAdminI()
{
  this->stop();
}


void
GenericSensorAdminI::start(const Ice::Current& current)
{
  this->nav_sensor_thread->watchSensor(this->sensor_id);
  printf("*** Watching sensor id %i\n", this->sensor_id);
}


void
GenericSensorAdminI::stop(const Ice::Current& current)
{
  this->nav_sensor_thread->unwatchSensor(this->sensor_id);
  printf("*** Unwatching sensor id %i\n", this->sensor_id);
}

void 
GenericSensorAdminI::setSensorReceiver(const vehicle::SensorFrameReceiverPrx &callback)
{
}
