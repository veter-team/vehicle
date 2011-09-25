/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "ReadCompassData.h"
#include <unistd.h>
#include <fcntl.h>
#include "i2c-from-u-boot/i2c_interface.h"

#define DELAY usleep(1 * 1000)

void 
InitCompassSensor()
{
  DELAY;

  // Read CMPS03 software version
  // query the register 0
  int version = i2c_reg_read(CMPS03ADDRESS, 0);
  if(version <= 0)
  {
     fprintf(stderr, "Compass initialization\nFailed to read from I2C compass device: %m\n");
     return;
  }
  printf("Compass software version: %i\n", version);
}


void 
ReadCompassData(vehicle::SensorData &data)
{
  DELAY;

  // Read bearing
  // query the register 1
  int bearing = i2c_reg_read(CMPS03ADDRESS, 1);
  if(bearing < 0)
  {
     fprintf(stderr, "Failed to read bearing from I2C compass device: %m\n");
     return;
  }

  data.intdata.push_back(bearing);
}

