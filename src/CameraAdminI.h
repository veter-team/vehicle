/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __CAMERAADMINI_H
#define __CAMERAADMINI_H


#include <IceE/Properties.h>
#include <vehicle.h>
#include <vehicleadmin.h>
#include "VideoSenderThread.h"


class CameraAdminI : public vehicleadmin::Admin
{
 public:
  CameraAdminI(const Ice::PropertiesPtr &properties, int ac, char **av);
  virtual ~CameraAdminI();

 public:
  virtual void start(const Ice::Current& = ::Ice::Current());
  virtual void stop(const Ice::Current& = ::Ice::Current());

  void setSensorReceiver(const vehicle::SensorFrameReceiverPrx &callback);

 private:
  int argc;
  char **argv;
  VideoSenderThreadPtr video_sender_thread;
  vehicle::SensorFrameReceiverPrx video_callback;
  IceUtil::ThreadControl vst_ctl;
  std::string encoding_pipeline;
};

typedef IceUtil::Handle<CameraAdminI> CameraAdminIPtr;

#endif // __CAMERAADMINI_H
