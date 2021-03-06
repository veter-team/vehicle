/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "ServoAdmin.h"


ServoAdmin::ServoAdmin()
{
}


void 
ServoAdmin::setServoGroup(ServoGroupPtr &sg)
{
  this->servo_group = sg;
}


void 
ServoAdmin::start(const ::Ice::Current&)
{
  this->servo_group->setStarted(true);
}


void 
ServoAdmin::stop(const ::Ice::Current&)
{
  this->servo_group->setStarted(false);
}
