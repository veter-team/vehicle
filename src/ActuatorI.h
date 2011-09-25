#ifndef __ACTUATORI_H
#define __ACTUATORI_H

#include <IceE/ObjectAdapter.h>
#include <vehicleadmin.h>
#include "ActuatorAdminI.h"

class ActuatorI : public vehicleadmin::Actuator
{
 public:
  ActuatorI(const vehicleadmin::ActuatorDescription &descr,
            Ice::ObjectAdapterPtr adm_adapter,
            short id);

 public:
    virtual vehicleadmin::AdminPrx getAdminInterface(const Ice::Current& = Ice::Current());
    virtual vehicleadmin::ActuatorDescription getDescription(const Ice::Current& = Ice::Current());

 private:
  vehicleadmin::ActuatorDescription description;
  short actuator_id;
  ActuatorAdminIPtr admin;
  vehicleadmin::AdminPrx adminprx;
};

typedef IceUtil::Handle<ActuatorI> ActuatorIPtr;

#endif // __ACTUATORI_H
