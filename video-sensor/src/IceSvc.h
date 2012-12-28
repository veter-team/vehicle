#ifndef __ICESVC_H
#define __ICESVC_H

#include <Ice/Service.h>
#include "SensorGroupI.h"


class IceSvc : public Ice::Service
{
 public:
  IceSvc();

 public:
  virtual bool start(int argc, char *argv[], int &status);
  virtual bool stop();

 private:
  SensorGroupIPtr sensor_group;
};


#endif // __ICESVC_H
