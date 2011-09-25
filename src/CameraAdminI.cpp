/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "CameraAdminI.h"


CameraAdminI::CameraAdminI(const Ice::PropertiesPtr &properties,
               int ac, char **av)
: argc(ac), argv(av), video_sender_thread(NULL)
{
  this->encoding_pipeline = properties->getProperty("Encoding.Pipeline");
}


CameraAdminI::~CameraAdminI()
{
  Ice::Current c;
  this->stop(c);
}


void
CameraAdminI::start(const Ice::Current& current)
{
  this->video_sender_thread = 
    new VideoSenderThread(this->argc, 
                          this->argv,
                          this->encoding_pipeline);
  this->video_sender_thread->addVideoCallback(this->video_callback);
  if(!this->video_sender_thread->isAlive())
  {
    printf("Starting video sender thread...\n");
    this->vst_ctl = this->video_sender_thread->start();
  }
  this->video_sender_thread->startSending();
}


void
CameraAdminI::stop(const Ice::Current& current)
{
  this->video_sender_thread->stopSending();

  this->video_sender_thread->requestShutdown();
  fprintf(stdout, "Waiting for video sender thread to shutdown\n");
  this->vst_ctl.join();
  fprintf(stdout, "Video sender thread stoped\n");
  
  // Not a leak. Smart pointer will decrease the ref count 
  // and delete the object.
  this->video_sender_thread = NULL;
}

void 
CameraAdminI::setSensorReceiver(const vehicle::SensorFrameReceiverPrx &callback)
{
  if(callback == NULL)
  {
    this->stop();
    this->video_callback = NULL;
  }
  else
  {
    this->video_callback = callback;
    if(this->video_sender_thread != NULL)
      this->video_sender_thread->addVideoCallback(callback);
  }
}
