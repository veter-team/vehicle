/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __UPDATERTHREAD_H
#define __UPDATERTHREAD_H

#include <stdint.h>
#include <vector>
#include <IceUtil/Mutex.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Handle.h>
#include <sensors.h>


typedef std::vector<uint8_t> addrlist_t;


class UpdaterThreadBase : public IceUtil::Thread
{
 public:
  UpdaterThreadBase(sensors::SensorFrameReceiverPrx cb,
		    IceUtil::Mutex &datamutex,
		    IceUtil::Mutex &busmutex,
		    sensors::SensorFrame &sf,
		    const addrlist_t &addresslist,
		    useconds_t updateinterval);

  virtual void run(); // Thread entry point
  void requestStop(); // Instruct thread loop to quit

  // Updates sensor_frame.
  // Rreturns true if there are differences between old an new
  // values. Otherwise false.
  virtual void querySensors() = 0;

 protected:
  bool should_stop;
  sensors::SensorFrameReceiverPrx sensor_cb;
  IceUtil::Mutex &data_mutex;
  IceUtil::Mutex &bus_mutex;
  sensors::SensorFrame &sensor_frame;
  const addrlist_t &address_list;
  useconds_t update_interval;
};


class SonarUpdaterThread : public UpdaterThreadBase
{
 public:
  SonarUpdaterThread(sensors::SensorFrameReceiverPrx cb,
		     IceUtil::Mutex &datamutex,
		     IceUtil::Mutex &busmutex,
		     sensors::SensorFrame &sf,
		     const addrlist_t &addresslist);

  virtual void querySensors();
};


class CompassUpdaterThread : public UpdaterThreadBase
{
 public:
  CompassUpdaterThread(sensors::SensorFrameReceiverPrx cb,
		       IceUtil::Mutex &datamutex,
		       IceUtil::Mutex &busmutex,
		       sensors::SensorFrame &sf,
		       const addrlist_t &addresslist);

  virtual void querySensors();
};


#endif // __UPDATERTHREAD_H
