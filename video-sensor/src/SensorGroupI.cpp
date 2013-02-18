/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "SensorGroupI.h"
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <stdint.h>

#include <Ice/Service.h>

#include <gst/gst.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include "camctlutils.h"
#endif

using namespace std;

GST_DEBUG_CATEGORY (video_sensor_debug);


size_t
EnumCameras(const char *dev_name_tmpl)
{
  struct stat stats;
  char buff[1024];
  size_t i = 0;
  sprintf(buff, dev_name_tmpl, i);
  while((stat(buff, &stats)) == 0)
  {
    ++i;
    sprintf(buff, dev_name_tmpl, i);
  }

  return i;
}


SensorGroupI::SensorGroupI(admin::StatePrx &prx, 
			   int argc,
			   char *argv[],
			   const Ice::PropertiesPtr &properties)
  : state_prx(prx), 
    log(Ice::Service::instance()->communicator()->getLogger())
{
  // Initialize gstreamer
  const gchar *nano_str;
  guint major, minor, micro, nano;
  char buff[1024];

  gst_init(&argc, &argv);

  gst_version(&major, &minor, &micro, &nano);
  if(nano == 1)
    nano_str = "(CVS)";
  else if(nano == 2)
    nano_str = "(Prerelease)";
  else
    nano_str = "";
  sprintf(buff, 
	  "This program is linked against GStreamer %d.%d.%d %s",
	  major, minor, micro, nano_str);
  log->print(buff);

  GST_DEBUG_CATEGORY_INIT(video_sensor_debug, "video_sensor", 0,
			  "video camera(s) sensor");

  encoding_pipeline = properties->getProperty("Encoding.Pipeline");

  // Initialize camera
  string camera_settings = properties->getProperty("Camera.controls");
  std::string camera_devs = properties->getProperty("Camera.devices");
  if(camera_devs.empty())
    camera_devs = "/dev/video%i";
#ifndef WIN32
  struct stat stats;
  size_t i = 0;
  sprintf(buff, camera_devs.c_str(), i);
  while((stat(buff, &stats)) == 0)
    {
      int hdevice = OpenVideoDevice(buff);
      if(hdevice < 0)
        {
	  ostringstream err;
          err << "Could not open device file " 
	      << buff << " for initialization\n"
	      << "Provided settings (if any) will be ignored";
	  log->error(err.str());
        }
      else
        {
          ControlList ctls;
          ListControls(hdevice, ctls);
          if(!camera_settings.empty())
            SetControls(hdevice, ctls, camera_settings);
          CloseVideoDevice(hdevice);
        }
      
      ++i;
      sprintf(buff, camera_devs.c_str(), i);
    }
#endif
  if(i == 0)
    throw runtime_error(string("No cameras detected at ") + camera_devs);

  sensors::SensorDescription descr = 
    {0, 
     i == 1 ? sensors::Camera : sensors::StereoCamera, 
     0, 255, 30.0, "Video camera", "easycap"};
  this->sensors_descr.push_back(descr);
}


SensorGroupI::~SensorGroupI()
{
  this->requestStop();
}


void 
SensorGroupI::requestStop()
{
  if(this->updater_thread && this->updater_thread->isAlive())
    {
      this->updater_thread->requestStop();
      this->updater_thread->getThreadControl().join();
    }
}


void 
SensorGroupI::requestStart()
{
  if(this->updater_thread && this->updater_thread->isAlive())
    return;

  this->updater_thread = new UpdaterThread(this->encoding_pipeline,
					   this->sensor_cb,
					   this->log);
  this->updater_thread->start();
}


admin::StatePrx
SensorGroupI::getStateInterface(const Ice::Current&)
{
  return this->state_prx;
}


sensors::SensorDescriptionSeq
SensorGroupI::getSensorDescription(const Ice::Current&)
{
  return this->sensors_descr;
}


sensors::SensorFrame
SensorGroupI::getCurrentValues(const Ice::Current&)
{
  return sensors::SensorFrame();
}


bool
SensorGroupI::setSensorReceiver(const sensors::SensorFrameReceiverPrx& callback,
				const Ice::Current&)
{
  // Stopping updater thread. We assume, that after changing receiver,
  // the State's start() method should becalled to resume sending
  // joystick data
  this->requestStop();
  this->sensor_cb = sensors::SensorFrameReceiverPrx::uncheckedCast(callback->ice_oneway()->ice_timeout(2000));
  return true;
}


void
SensorGroupI::cleanSensorReceiver(const Ice::Current&)
{
  this->requestStop();
  this->sensor_cb = 0;
}
