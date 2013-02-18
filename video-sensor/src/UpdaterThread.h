/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __UPDATERTHREAD_H
#define __UPDATERTHREAD_H

#include <IceUtil/Thread.h>
#include <IceUtil/Handle.h>
#include <Ice/Logger.h>

#include <sensors.h>

#include <gst/gst.h>


class UpdaterThread : public IceUtil::Thread
{
 public:
  UpdaterThread(const std::string &encpipeline,
		sensors::SensorFrameReceiverPrx cb,
		Ice::LoggerPtr l);

  virtual void run(); // Thread entry point
  void requestStop(); // Instruct thread loop to quit

  void sendBuffer(GstBuffer *buffer);

 private:
  //void setFrameSize(int new_width, int new_height);
  //void setBitrate(size_t new_rate);

  const std::string encoding_pipeline;
  sensors::SensorFrameReceiverPrx sensor_cb;
  Ice::LoggerPtr log;
  GstPipeline* pipeline;
  GMainLoop *loop;
  GstBus *bus;
};

typedef IceUtil::Handle<UpdaterThread> UpdaterThreadPtr;


#endif // __UPDATERTHREAD_H
