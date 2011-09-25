/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include <IceE/IceE.h>
#include <iostream>
#include "RemoteVehicleI.h"

using namespace std;


int 
run(int argc, char **argv, Ice::CommunicatorPtr& communicator)
{
  int retval = 0;

  // Prefere to do it once during system boot
  //system("devmem2 0x48002176 h 0x0002");
  //system("devmem2 0x48002178 h 0x0002");

  Ice::ObjectAdapterPtr vehicle_adapter = 
    communicator->createObjectAdapter("control");
  Ice::ObjectAdapterPtr admin_adapter = 
    communicator->createObjectAdapter("vehicleadmin");

  Ice::PropertiesPtr properties = communicator->getProperties();

  RemoteVehicleIPtr unit = new RemoteVehicleI(admin_adapter, vehicle_adapter, properties, argc, argv);
  vehicle_adapter->add(unit, communicator->stringToIdentity("unit"));
  cout << "  unit interface created" << endl;

  vehicle_adapter->activate();
  admin_adapter->activate();

  // Wait until we are done
  cout << "Initialization complete. Ready to process requests." << endl;

  communicator->waitForShutdown();

  cout << "shutdown sequence complete" << endl;
  return retval;
}


int 
main(int argc, char *argv[])
{
  int status;
  Ice::CommunicatorPtr communicator;

  try
    {
      cout << "Initializing communicator" << endl;
      communicator = Ice::initialize(argc, argv);
      status = run(argc, argv, communicator);
    }
  catch(const Ice::Exception& ex)
    {
      cerr << ex.toString() << endl;
      status = EXIT_FAILURE;
    }

  if(communicator)
    {
      try
        {
          communicator->destroy();
        }
      catch(const Ice::Exception& ex)
        {
          cerr << ex.toString() << endl;
          status = EXIT_FAILURE;
        }
    }

  cout << "Done." << endl;
  return status;
}
