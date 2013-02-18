/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "WheelsAdmin.h"


WheelsAdmin::WheelsAdmin(TracksController *tc)
  : tracks_ctl(tc)
{
}


void 
WheelsAdmin::start(const ::Ice::Current&)
{
  this->tracks_ctl->init();
}


void 
WheelsAdmin::stop(const ::Ice::Current&)
{
  this->tracks_ctl->uninit();
}
