/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __NAVSENSORTHREAD_H
#define __NAVSENSORTHREAD_H

#include <list>

#include <IceE/Thread.h>
#include <IceE/Mutex.h>
#include <vehicle.h>

#include "SensorActuatorIDs.h"


class NavSensorThread : public IceUtil::Thread
{
 public:
  NavSensorThread();
  ~NavSensorThread();

 public:
  virtual void run();
  void requestShutdown();
  void watchSensor(SensorID sensorid);
  void unwatchSensor(SensorID sensorid);
  void setCallback(const vehicle::SensorFrameReceiverPrx &callback);

 protected:
  bool shutdown_requested;
  typedef IceUtil::Mutex SyncT;
  SyncT sync;

  vehicle::SensorFrame sensor_data;
  vehicle::SensorFrameReceiverPrx sensor_callback;
};

typedef IceUtil::Handle<NavSensorThread> NavSensorThreadPtr;

#endif // __NAVSENSORTHREAD_H
