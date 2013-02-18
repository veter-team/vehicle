/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __ICESVC_H
#define __ICESVC_H

#include <Ice/Service.h>
#include "ServoAdmin.h"


class IceSvc : public Ice::Service
{
 public:
  IceSvc();

 public:
  virtual bool start(int argc, char *argv[], int &status);
  virtual bool stop();

 private:
  ServoAdminPtr servo_admin;
};


#endif // __ICESVC_H
