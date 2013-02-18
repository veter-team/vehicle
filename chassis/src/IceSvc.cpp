/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include <sys/mman.h>
#include "IceSvc.h"
#include "WheelsGroup.h"
#include "WheelsAdmin.h"

using namespace std;


IceSvc::IceSvc(TracksController *tc)
  : tracks_ctl(tc)
{
}


bool 
IceSvc::start(int argc, char *argv[], int &status)
{
  // Avoids memory swapping for this program
  if(0 != mlockall(MCL_CURRENT | MCL_FUTURE))
    {
      perror("mlockall() failed: ");
      fprintf(stderr, "errno = %d.\n", errno);      
      status = EXIT_FAILURE;
      return false;
    }

  Ice::ObjectAdapterPtr adapter = 
    this->communicator()->createObjectAdapter("chassis");

  admin::StatePtr adm = new WheelsAdmin(this->tracks_ctl);
  Ice::ObjectPrx obj = 
    adapter->add(adm, this->communicator()->stringToIdentity("wheels-admin"));
  admin::StatePrx state_prx = admin::StatePrx::uncheckedCast(obj);

  actuators::ActuatorGroupPtr wg = new WheelsGroup(state_prx, this->tracks_ctl);
  adapter->add(wg, this->communicator()->stringToIdentity("wheels"));

  adapter->activate();

  status = EXIT_SUCCESS;
  return true;
}


bool 
IceSvc::stop()
{
  this->tracks_ctl->uninit();
  return Ice::Service::stop();
}
