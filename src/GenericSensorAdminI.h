/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __GENERICSENSORADMINI_H
#define __GENERICSENSORADMINI_H


#include <IceE/Properties.h>
#include <vehicle.h>
#include <vehicleadmin.h>
#include "NavSensorThread.h"


class GenericSensorAdminI : public vehicleadmin::Admin
{
 public:
  GenericSensorAdminI(const Ice::PropertiesPtr &properties,
                      NavSensorThreadPtr nst,
                      SensorID sid);
  virtual ~GenericSensorAdminI();

 public:
  virtual void start(const Ice::Current& = ::Ice::Current());
  virtual void stop(const Ice::Current& = ::Ice::Current());

  void setSensorReceiver(const vehicle::SensorFrameReceiverPrx &callback);

 private:
  NavSensorThreadPtr nav_sensor_thread;
  SensorID sensor_id;
};

typedef IceUtil::Handle<GenericSensorAdminI> GenericSensorAdminIPtr;

#endif // __GENERICSENSORADMINI_H
