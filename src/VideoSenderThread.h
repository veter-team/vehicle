/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __VIDEOSENDERTHREAD_H
#define __VIDEOSENDERTHREAD_H

#include <list>

#include <gst/gst.h>

#include <IceE/Thread.h>
#include <IceE/Mutex.h>
#include <vehicle.h>
#include "VideoSenderThread.h"
#include "AMI_SensorFrameReceiver_nextSensorFrameI.h"
#include "Statystic.h"


class VideoSenderThread : public IceUtil::Thread
{
 public:
  VideoSenderThread(int ac, char **av,
                    const std::string &pipeline);
  ~VideoSenderThread();

 public:
  virtual void run();

  void addVideoCallback(const vehicle::SensorFrameReceiverPrx &cb);
  void startSending();
  void stopSending();
  void handleIceException(const Ice::Exception &ex);
  void requestShutdown();
  void sendBuffer(GstBuffer *buffer);
  void returnCBToPool(const AMI_SensorFrameReceiver_nextSensorFrameIPtr &cb);

 private:
  void changePipelineState(GstState state);
  // Implementation of the thread-safe interface pattern
  void stopSending_i();
  AMI_SensorFrameReceiver_nextSensorFrameIPtr getCBFromPool();
  void setFrameSize(int new_width, int new_height);
  void setBitrate(size_t new_rate);

  int argc;
  char **argv;
  std::string encoding_pipeline;
  vehicle::SensorFrameReceiverPrx callback;
  Statystic stat;
  typedef IceUtil::Mutex PoolSyncT;
  typedef std::list<AMI_SensorFrameReceiver_nextSensorFrameIPtr> CallbackPool;
  PoolSyncT pool_sync;
  CallbackPool ami_callbacks;
  bool sending;
  bool shutdown_requested;
  typedef IceUtil::Mutex SyncT;
  SyncT cb_sync;
  GstPipeline* pipeline;
  GstElement *appsrc;
  GMainLoop *loop;
  GstBus *bus;
  time_t start_time;
  size_t frame_width;
  size_t frame_height;
  int qos_state;
};

typedef IceUtil::Handle<VideoSenderThread> VideoSenderThreadPtr;

#endif // __VIDEOSENDERTHREAD_H
