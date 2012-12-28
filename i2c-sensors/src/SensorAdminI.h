#ifndef __SENSORADMINI_H
#define __SENSORADMINI_H

#include <admin.h>


template <typename SensorGroupIPtr>
class SensorAdminI : public admin::State
{
 public:

  SensorAdminI() {}

  void setSensorGroup(SensorGroupIPtr &sg)
  {
    this->sensor_group = sg;
  }

 public:

  virtual void start(const Ice::Current& = ::Ice::Current())
  {
    this->sensor_group->requestStart();
  }

  virtual void stop(const Ice::Current& = ::Ice::Current())
  {
    this->sensor_group->requestStop();
  }

 private:
  SensorGroupIPtr sensor_group;
};


#endif // __SENSORADMINI_H
