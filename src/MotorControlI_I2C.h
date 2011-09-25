/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __MOTORCONTROLI_H
#define __MOTORCONTROLI_H

#include <IceE/Properties.h>
#include <vehicle.h>


class MotorControlI : virtual public vehicle::MotorControl
{
 public:
  MotorControlI(const Ice::PropertiesPtr &properties);

 public:
  virtual void setDuties(const ::vehicle::MotorDutySeq &duties,
                         const Ice::Current&);

 private:
  int fd;
};

typedef IceUtil::Handle<MotorControlI> MotorControlIPtr;


#endif // __MOTORCONTROLI_H
