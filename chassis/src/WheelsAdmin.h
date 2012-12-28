#ifndef __WHEELSADMIN_H
#define __WHEELSADMIN_H

#include <admin.h>
#include "TracksController.h"

class WheelsAdmin : public admin::State
{
 public:
  WheelsAdmin(TracksController *tc);

 public:
  virtual void start(const ::Ice::Current& = ::Ice::Current());
  virtual void stop(const ::Ice::Current& = ::Ice::Current());

 private:
  TracksController *tracks_ctl;
};


#endif // __WHEELSADMIN_H
