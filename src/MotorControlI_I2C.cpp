/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "MotorControlI_I2C.h"
#include <iostream>
#include <fcntl.h>
#ifndef WIN32
#include "i2c-dev.h"
#endif

// 0x40 >> 1 = 0x20
#define ADDRESS 0x20


MotorControlI::MotorControlI(const Ice::PropertiesPtr &properties)
{
#ifndef WIN32
  this->fd = open("/dev/i2c-2", O_RDWR);
  if(this->fd == -1)
  {
    fprintf(stderr, "Failed to open /dev/i2c-2 file\n");
  }

  if(ioctl(this->fd, I2C_SLAVE, ADDRESS) < 0)
  {
    fprintf(stderr, "Failed to set slave address: %m\n");
  }
#endif
}


void
MotorControlI::setDuties(const vehicle::MotorDutySeq& duties,
                         const Ice::Current& current)
{
#ifndef WIN32
  __u8 command[2];
#endif

  std::cout << "Setting motor duties: " << std::endl;
  for(vehicle::MotorDutySeq::const_iterator d = duties.begin();
        d != duties.end(); ++d)
    {
      std::cout << "  motor " << d->motorId;
      std::cout << " -> " << d->duty << std::endl;
#ifndef WIN32
      command[0] = d->motorId;
      command[1] = d->duty;
      if(i2c_smbus_write_block_data(this->fd, 1, sizeof(command) / sizeof(command[0]), command) < 0)
        {
          fprintf(stderr, "Failed to write to I2C device: %m\n");
        }
#endif
    }
}

