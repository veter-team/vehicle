#ifndef __SENSORADMINI_H
#define __SENSORADMINI_H

#include <IceUtil/Handle.h>
#include <admin.h>
#include "SensorGroupI.h"


class SensorAdminI : public admin::State
{
 public:
  SensorAdminI();

  void setSensorGroup(SensorGroupIPtr &sg);

 public:
  virtual void start(const Ice::Current& = ::Ice::Current());
  virtual void stop(const Ice::Current& = ::Ice::Current());

 private:
  SensorGroupIPtr sensor_group;
};

typedef IceUtil::Handle<SensorAdminI> SensorAdminIPtr;

#endif // __SENSORADMINI_H
