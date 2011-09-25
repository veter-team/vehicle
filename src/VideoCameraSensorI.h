#ifndef __VIDEOCAMERASENSORI_H
#define __VIDEOCAMERASENSORI_H

#include <IceE/Properties.h>
#include <IceE/ObjectAdapter.h>
#include <vehicleadmin.h>
#include "CameraAdminI.h"


class VideoCameraSensorI : public vehicleadmin::Sensor
{
 public:
  VideoCameraSensorI(const Ice::PropertiesPtr &properties,
                     Ice::ObjectAdapterPtr adm_adapter,
                     int ac, char **av, 
                     const char *dev_file_name_template);
  virtual ~VideoCameraSensorI();

 public:
  virtual vehicleadmin::AdminPrx getAdminInterface(const Ice::Current& = Ice::Current());
  virtual vehicleadmin::SensorDescription getDescription(const Ice::Current& = Ice::Current());
  virtual vehicleadmin::CalibrationData getCalibrationData(const Ice::Current& = Ice::Current());
  virtual void setCalibrationData(const vehicleadmin::CalibrationData &calib, 
                                  const Ice::Current& = Ice::Current());

  // returns true if camera is present
  bool isPresent() const;

  void setSensorReceiver(const vehicle::SensorFrameReceiverPrx &callback);

 private:
  std::string dev_name_tmpl;
  CameraAdminIPtr admin;
  vehicleadmin::AdminPrx adminprx;
  vehicleadmin::CalibrationData calibration_data;
};

typedef IceUtil::Handle<VideoCameraSensorI> VideoCameraSensorIPtr;


#endif // __VIDEOCAMERASENSORI_H
