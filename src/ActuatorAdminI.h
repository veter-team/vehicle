/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __ACTUATORADMINI_H
#define __ACTUATORADMINI_H

#include <vehicleadmin.h>


class ActuatorAdminI : public vehicleadmin::Admin
{
 public:
  ActuatorAdminI();
  virtual ~ActuatorAdminI();

 public:
  virtual void start(const Ice::Current& = ::Ice::Current());
  virtual void stop(const Ice::Current& = ::Ice::Current());

 private:
};

typedef IceUtil::Handle<ActuatorAdminI> ActuatorAdminIPtr;

#endif // __ACTUATORADMINI_H
