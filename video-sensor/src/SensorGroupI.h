/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __SENSORGROUPI_H
#define __SENSORGROUPI_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Handle.h>
#include <Ice/Properties.h>
#include <Ice/Logger.h>

#include <sensors.h>

#include "UpdaterThread.h"


class SensorGroupI : public sensors::SensorGroup
{
 public:
  SensorGroupI(admin::StatePrx &prx,
	       int argc,
	       char *argv[],
	       const Ice::PropertiesPtr &properties);
  ~SensorGroupI();

 public:
  void requestStart();
  void requestStop();

 public:
  virtual admin::StatePrx getStateInterface(const Ice::Current& = Ice::Current());

  virtual sensors::SensorDescriptionSeq getSensorDescription(const Ice::Current& = Ice::Current());

  virtual sensors::SensorFrame getCurrentValues(const Ice::Current& = Ice::Current());

  virtual bool setSensorReceiver(const sensors::SensorFrameReceiverPrx& callback,
				 const Ice::Current& = Ice::Current());

  virtual void cleanSensorReceiver(const Ice::Current& = Ice::Current());

 private:
  admin::StatePrx state_prx;
  std::string encoding_pipeline;
  sensors::SensorFrameReceiverPrx sensor_cb;
  Ice::LoggerPtr log;
  sensors::SensorDescriptionSeq sensors_descr;
  UpdaterThreadPtr updater_thread;
};

typedef IceUtil::Handle<SensorGroupI> SensorGroupIPtr;


#endif // __SENSORGROUPI_H
