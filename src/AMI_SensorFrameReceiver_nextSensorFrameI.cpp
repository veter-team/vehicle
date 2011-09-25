/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "AMI_SensorFrameReceiver_nextSensorFrameI.h"
#include "Statystic.h"
#include "VideoSenderThread.h"


AMI_SensorFrameReceiver_nextSensorFrameI::AMI_SensorFrameReceiver_nextSensorFrameI(VideoSenderThread *t, 
                                                                                   Statystic *s)
  : frame_size(0), video_sender_thread(t), stat(s)
{
}


void 
AMI_SensorFrameReceiver_nextSensorFrameI::ice_response()
{
  fprintf(stderr, "Got unexpected response from one-way operation\n");
  stat->frameSent(0);
  this->video_sender_thread->returnCBToPool(this);
}


void 
AMI_SensorFrameReceiver_nextSensorFrameI::ice_exception(const Ice::Exception &e)
{
  this->video_sender_thread->handleIceException(e);
  stat->frameSent(0);
  this->video_sender_thread->returnCBToPool(this);
}


void 
AMI_SensorFrameReceiver_nextSensorFrameI::ice_sent()
{
  stat->frameSent(this->frame_size);
  this->video_sender_thread->returnCBToPool(this);
}
