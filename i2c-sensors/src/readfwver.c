/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "from-u-boot/i2c_interface.h"
#include "from-u-boot/enable_i2c_clocks.h"

//#define SONAR (0xE0 >> 1)
//#define SONAR (0xE2 >> 1)
//#define SONAR (0xE4 >> 1)
#define SONAR (0xE6 >> 1)
#define COMPAS (0xC0 >> 1)


int 
main(int argc, char **argv)
{
  size_t i;
  int version;
  int res = enable_i2c_clocls();
  if(res)
    {
      printf("Error enabling I2C clocks: %i\n", res);
      return res;
    }

  i2c_init(100000, 1);
  printf("I2C-2 initialized\n");
  usleep(100 * 1000);
  for(i = 0; i < 100; ++i)
  {
    version = i2c_reg_read(SONAR, 0);
    printf("Sonar firmware version: %i\n", version);
    version = 0;
    //usleep(100 * 1000);
    //version = i2c_reg_read(COMPAS, 0);
    //printf("Compass firmware version: %i\n", version);
    //version = 0;
    usleep(500 * 1000);
  }
  i2c_uninit();
  return 0;
}

