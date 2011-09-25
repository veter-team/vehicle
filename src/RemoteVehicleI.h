/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __REMOTEVEHICLE_H
#define __REMOTEVEHICLE_H

#include <IceE/Properties.h>
#include <vector>
#include <vehicle.h>
#include "VideoCameraSensorI.h"
#include "ActuatorI.h"
#include "MotorControlI_PWM.h"
#include "ServoControl.h"
#include "NavSensorThread.h"
#include "GenericSensorI.h"


class RemoteVehicleI : public vehicle::RemoteVehicle
{
 public:
  RemoteVehicleI(Ice::ObjectAdapterPtr adm_adapter,
                 Ice::ObjectAdapterPtr ctl_adapter,
                 const Ice::PropertiesPtr &properties,
                 int ac, char **av);
  ~RemoteVehicleI();

 public:

  virtual std::string getVehicleDescription(const Ice::Current& = Ice::Current());
  virtual vehicleadmin::SensorList getSensorList(const Ice::Current& = Ice::Current());
  virtual vehicleadmin::ActuatorList getActuatorList(const Ice::Current& = Ice::Current());

  virtual void setActuators(const vehicle::ActuatorFrame &duties, 
                            const ::Ice::Current& = ::Ice::Current());
  virtual void setSensorReceiver(const vehicle::SensorFrameReceiverPrx &callback, 
                                 const ::Ice::Current& = ::Ice::Current());
  virtual void cleanSensorReceiver(const ::Ice::Current& = ::Ice::Current());

 private:
  MotorControlI motor_control;
  ServoControl servo_control;
  Ice::ObjectAdapterPtr admin_adapter;
  Ice::ObjectAdapterPtr control_adapter;

  typedef std::vector< std::pair<VideoCameraSensorIPtr, vehicleadmin::SensorPrx> > CameraList;
  CameraList cameras;

  typedef std::vector< std::pair<GenericSensorIPtr, vehicleadmin::SensorPrx> > GenericSensorList;
  GenericSensorList generic_sensors;

  typedef std::vector< std::pair<ActuatorIPtr, vehicleadmin::ActuatorPrx> > ActuatorList;
  ActuatorList actuators;
  NavSensorThreadPtr nav_sensor_thread;
  IceUtil::ThreadControl nst_ctl;
};

typedef IceUtil::Handle<RemoteVehicleI> RemoteVehicleIPtr;


#endif // __REMOTEVEHICLE_H
