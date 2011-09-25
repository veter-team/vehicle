#include "GenericSensorI.h"
#include <sys/types.h>
#include <sys/stat.h>


GenericSensorI::GenericSensorI(const Ice::PropertiesPtr &properties, 
                               Ice::ObjectAdapterPtr adm_adapter,
                               NavSensorThreadPtr nst,
                               const char *prop_name)
{
  // Initialize camera
  const std::string prop(prop_name);
  this->description = 
    properties->getProperty(std::string("Sensors.") + prop + ".description");
  this->vendorid = 
    properties->getProperty(std::string("Sensors.") + prop + ".vendorid");
  std::string type_name = 
    properties->getProperty(std::string("Sensors.") + prop + ".type");
  std::transform(type_name.begin(), type_name.end(), 
                 type_name.begin(), tolower);
  if(type_name == "compass")
  {
    this->sensor_id = COMPASS;
    this->type = vehicleadmin::Compass;
  }
  else if(type_name == "range")
  {
    this->sensor_id = SONAR;
    this->type = vehicleadmin::Range;
  }
  else if(type_name == "gps")
  {
    this->sensor_id = GPS;
    this->type = vehicleadmin::GPS;
  }

  this->admin = new GenericSensorAdminI(properties, nst, this->sensor_id);
  this->adminprx = 
    vehicleadmin::AdminPrx::uncheckedCast(adm_adapter->addWithUUID(this->admin));
}


GenericSensorI::~GenericSensorI()
{
  this->admin->stop();
}


vehicleadmin::AdminPrx 
GenericSensorI::getAdminInterface(const Ice::Current &current)
{
  return this->adminprx;
}


vehicleadmin::SensorDescription 
GenericSensorI::getDescription(const Ice::Current &current)
{
  vehicleadmin::SensorDescription descr;
  descr.id = this->sensor_id;
  descr.description = this->description;
  descr.vendorid = this->vendorid;
  descr.type = this->type;
  descr.minvalue = 0; // data range
  descr.maxvalue = 0; // both values should be 0 if range is undefined
  return descr;
}


vehicleadmin::CalibrationData 
GenericSensorI::getCalibrationData(const Ice::Current &current)
{
  return this->calibration_data;
}


void 
GenericSensorI::setCalibrationData(const vehicleadmin::CalibrationData &calib, 
                                       const Ice::Current &current)
{
  this->calibration_data = calib;
}


void 
GenericSensorI::setSensorReceiver(const vehicle::SensorFrameReceiverPrx &callback)
{
  this->admin->setSensorReceiver(callback);
}
