#include "ActuatorI.h"


ActuatorI::ActuatorI(const vehicleadmin::ActuatorDescription &descr,
                     Ice::ObjectAdapterPtr adm_adapter,
                     short id)
  : description(descr), actuator_id(id)
{
  this->admin = new ActuatorAdminI();
  this->adminprx = 
    vehicleadmin::AdminPrx::uncheckedCast(adm_adapter->addWithUUID(this->admin));
}


vehicleadmin::AdminPrx 
ActuatorI::getAdminInterface(const Ice::Current &current)
{
  return this->adminprx;
}


vehicleadmin::ActuatorDescription 
ActuatorI::getDescription(const Ice::Current &current)
{
  return this->description;
}
