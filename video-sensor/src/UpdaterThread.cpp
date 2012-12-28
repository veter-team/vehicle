#include "UpdaterThread.h"

#include <Ice/Service.h>

#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>


static void 
changePipelineState(GstPipeline *pipeline, GstBus *bus, GstState state)
{
  GstStateChangeReturn ret =
    gst_element_set_state(GST_ELEMENT(pipeline), state);
  if(ret == GST_STATE_CHANGE_FAILURE)
    {
      Ice::LoggerPtr log = 
	Ice::Service::instance()->communicator()->getLogger();
      log->error("Failed to change pipeline state!");

      /* check if there is an error message with details on the bus */
      GstMessage* msg = gst_bus_poll(bus, GST_MESSAGE_ERROR, 0);
      if(msg)
        {
	  char buff[1024];
          GError *err = NULL;
          gst_message_parse_error(msg, &err, NULL);
          sprintf(buff, "Reported gst error: %s", err->message);
	  log->error(buff);
          g_error_free(err);
          gst_message_unref(msg);
        }
    }
}

static gboolean
bus_message(GstBus *bus, GstMessage *message, UpdaterThread *ut)
{
  GST_DEBUG("gst got bus message %s",
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
	Ice::LoggerPtr log = 
	  Ice::Service::instance()->communicator()->getLogger();
	char buff[1024];
        GError *error;
        gchar *debug;

        gst_message_parse_error(message,
                                &error,
                                &debug);
        sprintf(buff, "Error message: %s", error->message);
	log->error(buff);
        sprintf(buff, "Debug message: %s", debug);
	log->error(buff);
        g_free(debug);

        ut->requestStop();
      }
      break;

    case GST_MESSAGE_EOS:
      ut->requestStop();
      break;

    default:
      break;
    }
  return TRUE;
}


// Called when the appsink notifies us that there is a new buffer
// ready for processing
static void
on_new_buffer(GstElement *element, UpdaterThread *ut)
{
  // get the buffer from appsink
  GstBuffer *buffer = gst_app_sink_pull_buffer(GST_APP_SINK(element));
  // Send buffer to the remote app source
  ut->sendBuffer(buffer);
  // we don't need the appsink buffer anymore
  gst_buffer_unref(buffer);
}


UpdaterThread::UpdaterThread(const std::string &encpipeline,
			     sensors::SensorFrameReceiverPrx cb, 
			     Ice::LoggerPtr l)
  : encoding_pipeline(encpipeline), 
    sensor_cb(cb), 
    log(l), 
    pipeline(NULL), 
    loop(NULL), 
    bus(NULL)
{
}


void 
UpdaterThread::requestStop()
{
  if(this->pipeline)
    changePipelineState(this->pipeline, this->bus, GST_STATE_NULL);

  if(this->loop)
    g_main_loop_quit(this->loop);
}

/*
void 
SensorGroupI::setFrameSize(int new_width, int new_height)
{
  GstElement *qos_scaler = gst_bin_get_by_name(GST_BIN(this->pipeline), "qos-scaler");
  GstElement *qos_caps = gst_bin_get_by_name(GST_BIN(this->pipeline), "qos-caps");

  if(!qos_scaler)
    log->warning("No videoscale element with name 'qos-scaler' found.");
  if(!qos_caps)
    log->warning("No capsfilter element with name 'qos-caps' found.");

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
    log->warning("Adaptation by adjusting frame size will not be possible.");

  if(qos_scaler)
    gst_object_unref(GST_OBJECT(qos_scaler));
  if(qos_caps)
    gst_object_unref(GST_OBJECT(qos_caps));
}


void 
UpdaterThread::setBitrate(size_t new_rate)
{
  GstElement *encoder = 
    gst_bin_get_by_name(GST_BIN(this->pipeline), "encoder");

  if(!encoder)
    log->warning("No encoder element with name 'encoder' found.");

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
    log->warning("Adaptation by adjusting bitrate will not be possible.");

  if(encoder)
    gst_object_unref(GST_OBJECT(encoder));
}
*/

void 
UpdaterThread::sendBuffer(GstBuffer *buffer)
{
  if(this->sensor_cb == 0)
    {
      log->error("sendBuffer() called but callback is null. Stoping pipeline");
      this->requestStop();
      return;
    }

  sensors::SensorData sensor_data;
  sensor_data.sensorid = 0;
  const size_t buf_size = GST_BUFFER_SIZE(buffer);
  sensor_data.bytedata.insert(sensor_data.bytedata.end(), 
			      GST_BUFFER_DATA(buffer), 
			      GST_BUFFER_DATA(buffer) + buf_size);
  sensors::SensorFrame sensor_frame;
  sensor_frame.push_back(sensor_data);

  try
    {
      // Send the buffer to remote app source
      this->sensor_cb->nextSensorFrame(sensor_frame);
    }
  catch(const Ice::Exception& ex)
    {
      //log->print("Error sending frame. Forgetting the callback receiver");

      // send end-of-stream
      gst_element_send_event(GST_ELEMENT(this->pipeline), gst_event_new_eos());
    }
}


void 
UpdaterThread::run()
{
  this->loop = g_main_loop_new(NULL, FALSE);

  // Create the pipeline. The actual string is specified in config file.
  this->pipeline =
    GST_PIPELINE(gst_parse_launch(this->encoding_pipeline.c_str(), NULL));

  this->bus = gst_pipeline_get_bus(GST_PIPELINE(this->pipeline));
  // add watch for messages
  gst_bus_add_watch(this->bus, (GstBusFunc)bus_message, this);

  // we use appsink in push mode, it sends us a signal when data is available
  // and we pull out the data in the signal callback. We want the appsink to
  // push as fast as it can, hence the sync=false
  GstElement *icesink = gst_bin_get_by_name(GST_BIN(this->pipeline), "icesink");
  g_object_set(G_OBJECT(icesink), "emit-signals", TRUE, "sync", FALSE, NULL);
  g_signal_connect(icesink, "new-buffer",
                   G_CALLBACK (on_new_buffer), this);

  gst_object_unref(icesink);

  changePipelineState(this->pipeline, this->bus, GST_STATE_PLAYING);

  g_main_loop_run(this->loop);
  
  g_main_loop_unref(this->loop);
  gst_object_unref(this->pipeline);
  gst_object_unref(this->bus);
}
