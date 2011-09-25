#include "VideoCameraSensorI.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include "camctlutils.h"
#endif


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


VideoCameraSensorI::VideoCameraSensorI(const Ice::PropertiesPtr &properties, 
                                       Ice::ObjectAdapterPtr adm_adapter,
                                       int ac, char **av,
                                       const char *dev_file_name_template)
: dev_name_tmpl(dev_file_name_template)
{
  // Initialize camera
  std::string camera_settings = properties->getProperty("Camera.controls");
#ifndef WIN32
  struct stat stats;
  char buff[1024];
  size_t i = 0;
  sprintf(buff, dev_file_name_template, i);
  while((stat(buff, &stats)) == 0)
    {
      int hdevice = OpenVideoDevice(buff);
      if(hdevice < 0)
        {
          fprintf(stderr, 
                  "Could not open device file %s for initialization\n"
                  "Provided settings (if any) will be ignored\n", buff);
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
      sprintf(buff, dev_file_name_template, i);
    }
#endif

  this->admin = new CameraAdminI(properties, ac, av);
  this->adminprx = 
    vehicleadmin::AdminPrx::uncheckedCast(adm_adapter->addWithUUID(this->admin));
}


VideoCameraSensorI::~VideoCameraSensorI()
{
  this->admin->stop();
}


vehicleadmin::AdminPrx 
VideoCameraSensorI::getAdminInterface(const Ice::Current &current)
{
  return this->adminprx;
}


vehicleadmin::SensorDescription 
VideoCameraSensorI::getDescription(const Ice::Current &current)
{
  vehicleadmin::SensorDescription descr;
  descr.id = 0; // sensor unique (for the vehicle) id
  descr.description = "Video camera"; // for example compass or gyroscope
  descr.vendorid = "Logitech 9000Pro"; // model id
  descr.type = (EnumCameras(this->dev_name_tmpl.c_str()) > 1 ? vehicleadmin::StereoCamera : vehicleadmin::Camera);
  descr.minvalue = 0; // data range
  descr.maxvalue = 0; // both values should be 0 if range is undefined
  return descr;
}


vehicleadmin::CalibrationData 
VideoCameraSensorI::getCalibrationData(const Ice::Current &current)
{
  return this->calibration_data;
}


void 
VideoCameraSensorI::setCalibrationData(const vehicleadmin::CalibrationData &calib, 
                                       const Ice::Current &current)
{
  this->calibration_data = calib;
}


bool 
VideoCameraSensorI::isPresent() const
{
  if(EnumCameras(this->dev_name_tmpl.c_str()) > 0)
    return true;

  return false;
}


void 
VideoCameraSensorI::setSensorReceiver(const vehicle::SensorFrameReceiverPrx &callback)
{
  this->admin->setSensorReceiver(callback);
}
