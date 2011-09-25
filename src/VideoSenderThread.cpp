/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "VideoSenderThread.h"

#include <IceE/IceE.h>

#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>

#include "SensorActuatorIDs.h"


#define INIT_FRAME_W 320
#define INIT_FRAME_H 240

GST_DEBUG_CATEGORY (vehicle_video_debug);
#define GST_CAT_DEFAULT vehicle_video_debug


static gboolean
bus_message(GstBus *bus, GstMessage *message, VideoSenderThread *thread)
{
  GST_DEBUG("got message %s",
            gst_message_type_get_name(GST_MESSAGE_TYPE(message)));

  switch(GST_MESSAGE_TYPE(message))
    {
    /*
    case GST_MESSAGE_QOS:
      g_print("QoS message received on the bus\n");
      break;
    */
    case GST_MESSAGE_ERROR:
      {
        GError *error;
        gchar *debug;

        gst_message_parse_error(message,
                                &error,
                                &debug);
        g_print("Error message: %s\n", error->message);
        g_print("Debug message: %s\n", debug);
        g_free(debug);

        g_error("received error");
        thread->requestShutdown();
      }
      break;

    case GST_MESSAGE_EOS:
      thread->requestShutdown();
      break;

    default:
      break;
    }
  return TRUE;
}


// Called when the appsink notifies us that there is a new buffer
// ready for processing
static void
on_new_buffer(GstElement *element, VideoSenderThread *thread)
{
  // get the buffer from appsink
  GstBuffer *buffer = gst_app_sink_pull_buffer(GST_APP_SINK(element));
  // Send buffer to the remote app source
  thread->sendBuffer(buffer);
  // we don't need the appsink buffer anymore
  gst_buffer_unref(buffer);
}


VideoSenderThread::VideoSenderThread(int ac, char **av, 
                                     const std::string &pipeline)
  : argc(ac),
    argv(av),
    encoding_pipeline(pipeline),
    callback(0),
    sending(false),
    shutdown_requested(false),
    frame_width(INIT_FRAME_W),
    frame_height(INIT_FRAME_H),
    qos_state(0)
{
  // Initialize gstreamer
  const gchar *nano_str;
  guint major, minor, micro, nano;

  gst_init(&(this->argc), &(this->argv));

  gst_version(&major, &minor, &micro, &nano);
  if(nano == 1)
    nano_str = "(CVS)";
  else if(nano == 2)
    nano_str = "(Prerelease)";
  else
    nano_str = "";
  printf("This program is linked against GStreamer %d.%d.%d %s\n",
         major, minor, micro, nano_str);

  GST_DEBUG_CATEGORY_INIT (vehicle_video_debug, "vehicle_video", 0,
      "vehicle video system");

  this->loop = g_main_loop_new(NULL, FALSE);

  // Create the pipeline. The actual string is specified in config file.
  this->pipeline =
    GST_PIPELINE(gst_parse_launch(this->encoding_pipeline.c_str(), NULL));

  this->bus = gst_pipeline_get_bus(GST_PIPELINE(this->pipeline));
  // add watch for messages
  gst_bus_add_watch(bus, (GstBusFunc)bus_message, this);

  // we use appsink in push mode, it sends us a signal when data is available
  // and we pull out the data in the signal callback. We want the appsink to
  // push as fast as it can, hence the sync=false
  GstElement *icesink = gst_bin_get_by_name(GST_BIN(this->pipeline), "icesink");
  g_object_set(G_OBJECT(icesink), "emit-signals", TRUE, "sync", FALSE, NULL);
  g_signal_connect(icesink, "new-buffer",
                   G_CALLBACK (on_new_buffer), this);
  gst_object_unref(icesink);
}


VideoSenderThread::~VideoSenderThread()
{
}


void 
VideoSenderThread::changePipelineState(GstState state)
{
  GstStateChangeReturn ret =
    gst_element_set_state(GST_ELEMENT(this->pipeline), state);
  if(ret == GST_STATE_CHANGE_FAILURE)
    {
      fprintf(stderr, "Failed to change pipeline state!");

      /* check if there is an error message with details on the bus */
      GstMessage* msg = gst_bus_poll(this->bus, GST_MESSAGE_ERROR, 0);
      if(msg)
        {
          GError *err = NULL;
          gst_message_parse_error(msg, &err, NULL);
          fprintf(stderr, "ERROR: %s", err->message);
          g_error_free(err);
          gst_message_unref(msg);
        }
    }
}


void 
VideoSenderThread::startSending()
{
  SyncT::Lock lock(this->cb_sync);
  if(!this->callback)
  {
      this->sending = false;
      return;
  }
  if(this->sending)
    return;

  this->sending = true;
  this->changePipelineState(GST_STATE_PLAYING);
  this->start_time = time(NULL);
}


void 
VideoSenderThread::stopSending()
{
  SyncT::Lock lock(this->cb_sync);
  this->stopSending_i();
}


void 
VideoSenderThread::stopSending_i()
{
  this->sending = false;
  this->changePipelineState(GST_STATE_NULL);

  PoolSyncT::Lock lock(this->pool_sync);
  this->ami_callbacks.clear();
  g_main_loop_quit(this->loop);
}


void 
VideoSenderThread::addVideoCallback(const vehicle::SensorFrameReceiverPrx &cb)
{
  SyncT::Lock lock(this->cb_sync);

  if(!cb && this->sending)
  {
    this->stopSending_i();
    return;
  }

  this->callback = 
    vehicle::SensorFrameReceiverPrx::uncheckedCast(cb->ice_oneway()->ice_timeout(2000));
}


void 
VideoSenderThread::requestShutdown()
{
  this->shutdown_requested = true;
  this->changePipelineState(GST_STATE_NULL);
}


void 
VideoSenderThread::handleIceException(const Ice::Exception &ex)
{
  fprintf(stderr, "Error sending video frame: %s\n",
          ex.toString().c_str());
  this->callback = 0;
  this->stopSending_i();
}


void 
VideoSenderThread::returnCBToPool(const AMI_SensorFrameReceiver_nextSensorFrameIPtr &cb)
{
  PoolSyncT::Lock lock(this->pool_sync);
  this->ami_callbacks.push_back(cb);
}


AMI_SensorFrameReceiver_nextSensorFrameIPtr 
VideoSenderThread::getCBFromPool()
{
  PoolSyncT::Lock lock(this->pool_sync);

  if(this->ami_callbacks.empty())
    return new AMI_SensorFrameReceiver_nextSensorFrameI(this, &(this->stat));

  AMI_SensorFrameReceiver_nextSensorFrameIPtr retval = this->ami_callbacks.back();
  this->ami_callbacks.pop_back();
  return retval;
}


void 
VideoSenderThread::setFrameSize(int new_width, int new_height)
{
  GstElement *qos_scaler = gst_bin_get_by_name(GST_BIN(this->pipeline), "qos-scaler");
  GstElement *qos_caps = gst_bin_get_by_name(GST_BIN(this->pipeline), "qos-caps");

  if(!qos_scaler)
    fprintf(stderr, "No videoscale element with name 'qos-scaler' found.\n");
  if(!qos_caps)
    fprintf(stderr, "No capsfilter element with name 'qos-caps' found.\n");

  if(qos_scaler && qos_caps)
  {
	  GstCaps *caps = gst_caps_new_empty();
	  GstStructure *cs;
	  cs = gst_structure_new("video/x-raw-yuv",
                  //format=(fourcc)UYVY
		  "width", G_TYPE_INT, new_width,
		  "height", G_TYPE_INT, new_height, NULL);

    gst_caps_append_structure(caps, cs);
    gst_element_set_state(qos_scaler, GST_STATE_NULL);
    gst_element_set_state(qos_caps, GST_STATE_NULL);
    g_object_set(G_OBJECT(qos_caps), "caps", caps, NULL);
	  gst_caps_unref(caps);
    gst_element_set_state(qos_scaler, GST_STATE_PLAYING);
    gst_element_set_state(qos_caps, GST_STATE_PLAYING);
  }
  else
    fprintf(stderr, "Adaptation by adjusting frame size is not be possible.\n");

  if(qos_scaler)
    gst_object_unref(GST_OBJECT(qos_scaler));
  if(qos_caps)
    gst_object_unref(GST_OBJECT(qos_caps));
}


void 
VideoSenderThread::setBitrate(size_t new_rate)
{
  GstElement *encoder = 
    gst_bin_get_by_name(GST_BIN(this->pipeline), "encoder");

  if(!encoder)
    fprintf(stderr, "No encoder element with name 'encoder' found.\n");

  if(encoder)
  {
    gst_element_set_state(encoder, GST_STATE_PAUSED);
    //GstPad *pad = gst_element_get_static_pad(encoder, "src");
    //gst_pad_set_blocked(pad, TRUE);
    g_object_set(G_OBJECT(encoder), "bitrate", gint(new_rate), NULL);
    //gst_pad_set_blocked(pad, FALSE);
    //gst_object_unref(GST_OBJECT(pad));
    gst_element_set_state(encoder, GST_STATE_PLAYING);
  }
  else
    fprintf(stderr, "Adaptation by adjusting bitrate is not be possible.\n");

  if(encoder)
    gst_object_unref(GST_OBJECT(encoder));
}


void 
VideoSenderThread::sendBuffer(GstBuffer *buffer)
{
  SyncT::Lock lock(this->cb_sync);
  if(!this->callback)
    return;
  try
    {
      vehicle::SensorData sensor_data;
      sensor_data.sensorid = CAMERA0;
      const size_t buf_size = GST_BUFFER_SIZE(buffer);
      sensor_data.bytedata.insert(sensor_data.bytedata.end(), 
                   GST_BUFFER_DATA(buffer), 
                   GST_BUFFER_DATA(buffer) + buf_size);
      // Send the buffer to remote app source
      AMI_SensorFrameReceiver_nextSensorFrameIPtr cb = this->getCBFromPool();
      cb->frame_size = buf_size;
      vehicle::SensorFrame sensor_frame;
      sensor_frame.push_back(sensor_data);

      if(this->callback->nextSensorFrame_async(cb, sensor_frame))
        {
          this->returnCBToPool(cb);
          this->stat.frameSent(buf_size);
        }
      else
        this->stat.frameQueued(buf_size);

      /*
        this is an experiment how to change the running pipeline
        to react on changing networking conditions.
        it should be actually triggered by statistic analyzer
      */
      int qos = this->stat.getQoSState();
      if(qos != this->qos_state)
        {
          // decrease frame size exponentially

          this->frame_width = INIT_FRAME_W / (1 << qos);
          this->frame_height = INIT_FRAME_H / (1 << qos);
          char strtime[128];
          time_t t = time(NULL);
          strftime(strtime, 
                   sizeof(strtime), 
                   "%x %X",
                   localtime(&t));
          printf("%s: set frame size to %u x %u\n",
                 strtime,
                 this->frame_width, this->frame_height);
          this->setFrameSize(this->frame_width, this->frame_height);

          // adjust bitrate
          /*
          size_t new_rate = 300 - (60 * qos);
          printf("Setting bitrate to %u\n", new_rate);
          this->setBitrate(new_rate); 
          */
          this->qos_state = qos;
        }
    }
  catch(const Ice::Exception& ex)
    {
      this->handleIceException(ex);
    }    
}


void 
VideoSenderThread::run()
{
  /* this mainloop is stopped when we receive an error or EOS */
  g_main_loop_run(this->loop);

  g_main_loop_unref(this->loop);
  gst_object_unref(this->bus);
}
