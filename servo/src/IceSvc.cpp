/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include <fcntl.h>
#include <sys/mman.h>

#include "IceSvc.h"
#include "ServoGroup.h"
#include "ServoAdmin.h"

using namespace std;

IceSvc::IceSvc()
{
}


bool 
IceSvc::start(int argc, char *argv[], int &status)
{
  // Pinmuxing:
  // pwm10 is gpio 145 in mode 2
  //system("devmem2 0x48002176 h 0x0002");
  // pwm11 is gpio 146 in mode 2
  //system("devmem2 0x48002178 h 0x0002");
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  if(fd < 0)
  {
    this->error("Could not open /dev/mem");
    status = EXIT_FAILURE;
    return false;
  }
  volatile uint8_t *pinconf =
    (uint8_t*)mmap(NULL, 0x10000, PROT_READ | PROT_WRITE,
		    MAP_SHARED, fd, 0x48000000);
  if(pinconf == MAP_FAILED)
  {
    this->error("Pinconf Mapping failed");
    close(fd);
    status = EXIT_FAILURE;
    return false;
  }
  *((volatile uint16_t*)(pinconf + 0x2176)) = 0x0002;
  *((volatile uint16_t*)(pinconf + 0x2178)) = 0x0002;
  close(fd);

  // Initializing servants
  Ice::ObjectAdapterPtr adapter = 
    this->communicator()->createObjectAdapter("servo");

  this->servo_admin = new ServoAdmin();
  Ice::ObjectPrx obj = 
    adapter->add(this->servo_admin, 
		 this->communicator()->stringToIdentity("servo-admin"));
  admin::StatePrx state_prx = admin::StatePrx::uncheckedCast(obj);

  ServoGroupPtr sg = new ServoGroup(state_prx);
  this->servo_admin->setServoGroup(sg);
  adapter->add(sg, this->communicator()->stringToIdentity("servo"));

  adapter->activate();

  status = EXIT_SUCCESS;
  return true;
}


bool 
IceSvc::stop()
{
  this->servo_admin->stop();
  return Ice::Service::stop();
}
