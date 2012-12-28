#ifndef __ICESVC_H
#define __ICESVC_H

#include <Ice/Service.h>
#include "TracksController.h"


class IceSvc : public Ice::Service
{
 public:
  IceSvc(TracksController *tc);

 public:
  virtual bool start(int argc, char *argv[], int &status);
  virtual bool stop();

 private:
  TracksController *tracks_ctl;
};


#endif // __ICESVC_H
