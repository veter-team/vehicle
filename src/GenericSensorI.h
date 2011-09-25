#ifndef __GENERICSENSORI_H
#define __GENERICSENSORI_H

#include <IceE/Properties.h>
#include <IceE/ObjectAdapter.h>
#include <vehicle.h>
#include <vehicleadmin.h>
#include "GenericSensorAdminI.h"
#include "NavSensorThread.h"
#include "SensorActuatorIDs.h"


class GenericSensorI : public vehicleadmin::Sensor
{
 public:
  GenericSensorI(const Ice::PropertiesPtr &properties,
                 Ice::ObjectAdapterPtr adm_adapter,
                 NavSensorThreadPtr nst,
                 const char *prop_name);
  virtual ~GenericSensorI();

 public:
  virtual vehicleadmin::AdminPrx getAdminInterface(const Ice::Current& = Ice::Current());
  virtual vehicleadmin::SensorDescription getDescription(const Ice::Current& = Ice::Current());
  virtual vehicleadmin::CalibrationData getCalibrationData(const Ice::Current& = Ice::Current());
  virtual void setCalibrationData(const vehicleadmin::CalibrationData &calib, 
                                  const Ice::Current& = Ice::Current());

  void setSensorReceiver(const vehicle::SensorFrameReceiverPrx &callback);

 private:
  std::string description;
  std::string vendorid;
  vehicleadmin::SensorType type;
  SensorID sensor_id;

  GenericSensorAdminIPtr admin;
  vehicleadmin::AdminPrx adminprx;
  vehicleadmin::CalibrationData calibration_data;
};

typedef IceUtil::Handle<GenericSensorI> GenericSensorIPtr;


#endif // __GENERICSENSORI_H
