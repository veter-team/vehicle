/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __AMI_SENSORFRAMERECEIVER_NEXTSENSORFRAMEI_H
#define __AMI_SENSORFRAMERECEIVER_NEXTSENSORFRAMEI_H

#include <vehicle.h>

class Statystic;
class VideoSenderThread;

class AMI_SensorFrameReceiver_nextSensorFrameI 
: public vehicle::AMI_SensorFrameReceiver_nextSensorFrame,
  public Ice::AMISentCallback
{
 public:
  AMI_SensorFrameReceiver_nextSensorFrameI(VideoSenderThread *t, Statystic *s);

 public:
  virtual void ice_response();
  virtual void ice_exception(const Ice::Exception &e);
  virtual void ice_sent();

  size_t frame_size;

 private:
  VideoSenderThread *video_sender_thread;
  Statystic *stat;
};

typedef IceUtil::Handle<AMI_SensorFrameReceiver_nextSensorFrameI> AMI_SensorFrameReceiver_nextSensorFrameIPtr;

#endif // __AMI_SENSORFRAMERECEIVER_NEXTSENSORFRAMEI_H
