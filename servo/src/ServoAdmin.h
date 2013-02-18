/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __WHEELSADMIN_H
#define __WHEELSADMIN_H

#include <admin.h>
#include "ServoGroup.h"


class ServoAdmin : public admin::State
{
 public:
  ServoAdmin();

  void setServoGroup(ServoGroupPtr &sg);

 public:
  virtual void start(const ::Ice::Current& = ::Ice::Current());
  virtual void stop(const ::Ice::Current& = ::Ice::Current());

 private:
  ServoGroupPtr servo_group;
};

typedef IceUtil::Handle<ServoAdmin> ServoAdminPtr;

#endif // __WHEELSADMIN_H
