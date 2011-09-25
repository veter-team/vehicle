/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "RemoteVehicleI.h"
#include <unistd.h>
#include "SensorActuatorIDs.h"


RemoteVehicleI::RemoteVehicleI(Ice::ObjectAdapterPtr adm_adapter,
                               Ice::ObjectAdapterPtr ctl_adapter,
                               const Ice::PropertiesPtr &properties,
                               int ac, char **av)
  : admin_adapter(adm_adapter), 
    control_adapter(ctl_adapter), 
    motor_control(properties),
    servo_control(properties, "cameraservo")
{
  this->nav_sensor_thread = new NavSensorThread();

  std::string camera_devs = properties->getProperty("Camera.devices");
  if(camera_devs.empty())
    camera_devs = "/dev/video%i";
  VideoCameraSensorIPtr camera0 = 
    new VideoCameraSensorI(properties, adm_adapter, ac, av, camera_devs.c_str());
  this->cameras.push_back(CameraList::value_type(camera0, vehicleadmin::SensorPrx(NULL)));
  
  // Add I2C compass and sonar
  GenericSensorList::value_type s;

  s.first = new GenericSensorI(properties, 
                               adm_adapter, 
                               this->nav_sensor_thread, 
                               "compass");
  s.second = vehicleadmin::SensorPrx::uncheckedCast(ctl_adapter->addWithUUID(s.first));
  this->generic_sensors.push_back(s);

  s.first = new GenericSensorI(properties, 
                               adm_adapter, 
                               this->nav_sensor_thread, 
                               "sonar1");
  s.second = vehicleadmin::SensorPrx::uncheckedCast(ctl_adapter->addWithUUID(s.first));
  this->generic_sensors.push_back(s);

  s.first = new GenericSensorI(properties, 
                               adm_adapter, 
                               this->nav_sensor_thread, 
                               "gps");
  s.second = vehicleadmin::SensorPrx::uncheckedCast(ctl_adapter->addWithUUID(s.first));
  this->generic_sensors.push_back(s);


  std::string prop;
  ActuatorList::value_type a;
  vehicleadmin::ActuatorDescription descr;

  // first motor
  descr.id = MOTOR0;
  prop = properties->getProperty("Actuators.motorl.description");
  descr.description = prop.empty() ? "Left motor" : prop;
  prop = properties->getProperty("Actuators.motorl.type");
  descr.type = prop.empty() ? "brush, 7.2V max" : prop;
  a.first = new ActuatorI(descr, adm_adapter, descr.id);
  a.second = vehicleadmin::ActuatorPrx::uncheckedCast(ctl_adapter->addWithUUID(a.first));
  this->actuators.push_back(a);

  // second motor
  descr.id = MOTOR1;
  prop = properties->getProperty("Actuators.motorr.description");
  descr.description = prop.empty() ? "Right motor" : prop;
  prop = properties->getProperty("Actuators.motorr.type");
  descr.type = prop.empty() ? "brush, 7.2V max" : prop;
  a.first = new ActuatorI(descr, adm_adapter, descr.id);
  a.second = vehicleadmin::ActuatorPrx::uncheckedCast(ctl_adapter->addWithUUID(a.first));
  this->actuators.push_back(a);

  // servo motor to rotate the camera
  descr.id = CAMERA_SERVO;
  prop = properties->getProperty("Actuators.cameraservo.description");
  descr.description = prop.empty() ? "Camera rotation" : prop;
  prop = properties->getProperty("Actuators.cameraservo.type");
  descr.type = prop.empty() ? "Futaba servo" : prop;
  a.first = new ActuatorI(descr, adm_adapter, descr.id);
  a.second = vehicleadmin::ActuatorPrx::uncheckedCast(ctl_adapter->addWithUUID(a.first));
  this->actuators.push_back(a);
}


RemoteVehicleI::~RemoteVehicleI()
{
  if(this->nav_sensor_thread)
  {
    this->nav_sensor_thread->requestShutdown();
    fprintf(stdout, "Waiting for nav sensor sender thread to shutdown\n");
    this->nst_ctl.join();
    
    // Not a leak. Smart pointer will decrease the ref count 
    // and delete the object.
    this->nav_sensor_thread = NULL;
  }
}


std::string 
RemoteVehicleI::getVehicleDescription(const Ice::Current &current)
{
  std::string descr("V-");
  char hostname[1024];
  int res = gethostname(hostname, sizeof(hostname)/sizeof(hostname[0]));
  if(res != 0)
    {// error obtaining host name
      descr += "unknown";
    }
  else
    {
      // just for the case if hostname is longer then our buffer
      hostname[sizeof(hostname)/sizeof(hostname[0]) - 1] = '\0';
      descr += hostname;
    }
  return descr;
}


vehicleadmin::SensorList 
RemoteVehicleI::getSensorList(const Ice::Current &current)
{
  vehicleadmin::SensorList lst;

  for(CameraList::iterator camera = this->cameras.begin();
      camera != this->cameras.end(); ++camera)
  {
    if(camera->first->isPresent())
    {
      if(camera->second == NULL)
        camera->second = vehicleadmin::SensorPrx::uncheckedCast(this->control_adapter->addWithUUID(camera->first));
      lst.push_back(camera->second);
    }
    else
      camera->second = NULL;
  }

  for(GenericSensorList::const_iterator sensor = this->generic_sensors.begin();
      sensor != this->generic_sensors.end(); ++sensor)
  lst.push_back(sensor->second);

  return lst;
}


vehicleadmin::ActuatorList 
RemoteVehicleI::getActuatorList(const Ice::Current &current)
{
  vehicleadmin::ActuatorList lst;
  for(ActuatorList::const_iterator actuator = this->actuators.begin();
      actuator != this->actuators.end(); ++actuator)
    lst.push_back(actuator->second);
  return lst;
}


void 
RemoteVehicleI::setActuators(const vehicle::ActuatorFrame &duties, 
                             const ::Ice::Current &current)
{
  vehicle::ActuatorFrame d;
  for(vehicle::ActuatorFrame::const_iterator i = duties.begin();
      i != duties.end(); ++i)
    {
      if(i->id == 2)
        this->servo_control.setDuties(*i);
      else
        d.push_back(*i);
    }

  if(!d.empty())
    this->motor_control.setDuties(d);
}


void 
RemoteVehicleI::setSensorReceiver(const vehicle::SensorFrameReceiverPrx &callback, 
                                  const ::Ice::Current &current)
{
  for(CameraList::const_iterator camera = this->cameras.begin();
      camera != this->cameras.end(); ++camera)
  {
    if(camera->first->isPresent())
      camera->first->setSensorReceiver(callback);
  }

  if(!this->nav_sensor_thread->isAlive())
  {
    printf("Starting nav sensor thread\n");
    this->nst_ctl = this->nav_sensor_thread->start();
  }

  this->nav_sensor_thread->setCallback(callback);
}


void 
RemoteVehicleI::cleanSensorReceiver(const ::Ice::Current &current)
{
  for(CameraList::const_iterator camera = this->cameras.begin();
      camera != this->cameras.end(); ++camera)
  {
    if(camera->first->isPresent())
      camera->first->setSensorReceiver(NULL);
  }

  this->nav_sensor_thread->setCallback(NULL);
  if(this->nav_sensor_thread->isAlive())
  {
    printf("Stoping nav sensor thread...\n");
    this->nav_sensor_thread->requestShutdown();
    this->nst_ctl.join();
    printf("Nav sensor thread stoped.\n");
  }
}

