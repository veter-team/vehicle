/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "ReadSonarData.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

//#define DELAY usleep(70)
#define DELAY


void 
InitSonarSensor()
{
/*o
  DELAY;

  if(ioctl(fd, I2C_SLAVE, US1ADDRESS) < 0)
  {
    fprintf(stderr, "Failed to set slave address for sonar: %m\n");
    return;
  }

  DELAY;

  // Read SRF08 software version
  // query the register 0
  if(i2c_smbus_write_byte(fd, 0) < 0)
  {
     fprintf(stderr, "Sonar initialization\nFailed to write to I2C device: %m\n");
     return;
  }

  DELAY;

  int version = i2c_smbus_read_byte(fd);

  DELAY;

  if(version < 0)
  {
     fprintf(stderr, "Sonar initialization\nFailed to read from I2C device: %m\n");
     return;
  }
  printf("Sonar software version: %i\n", version);
*/
}


void 
ReadSonarData(vehicle::SensorData &data)
{
/*
  DELAY;

  if(ioctl(fd, I2C_SLAVE, US1ADDRESS) < 0)
  {
    fprintf(stderr, "Failed to set slave address for sonar: %m\n");
    return;
  }

  usleep(500);

  // Read distance

  // Send Command Byte
  // Send 0x51 to start a ranging
  if(i2c_smbus_write_byte(fd, 0) < 0)
  {
     fprintf(stderr, "Failed to query distance (write 0): %m\n");
     return;
  }
  DELAY;
  if(i2c_smbus_write_byte(fd, 0x51) < 0)
  {
     fprintf(stderr, "Failed to query distance (write 0x51): %m\n");
     return;
  }

  // Wait for ranging to be complete
  usleep(100 * 1000);

  if(i2c_smbus_write_byte(fd, 0x02) < 0)
  {
     fprintf(stderr, "Failed to query distance (write 0x02): %m\n");
     return;
  }
  DELAY;
  uint32_t distance;
  uint8_t *d = (__u8*)&distance;
  d[1] = i2c_smbus_read_byte(fd);
  DELAY;
  d[0] = i2c_smbus_read_byte(fd);
  DELAY;
  d[2] = i2c_smbus_read_byte(fd);
  DELAY;
  d[3] = i2c_smbus_read_byte(fd);
  DELAY;

  if(distance < 0)
  {
     fprintf(stderr, "Failed to read distance from I2C sonar device: %m\n");
     return;
  }
  data.intdata.push_back(distance);
*/
}

