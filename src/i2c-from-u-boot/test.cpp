#include <stdio.h>
#include <unistd.h>
#include "i2c_interface.h"

int 
main(int argc, char **argv)
{
  i2c_init(100000, 1);
  usleep(100 * 1000);
  int version = i2c_reg_read(0x70, 0);
  printf("Sonar firmware version: %i\n", version);
  version = i2c_reg_read(0x60, 0);
  printf("Compass firmware version: %i\n", version);

  // Read 10 values from compass
  int bearing = 0;
  for(size_t i = 0; i < 10; ++i)
  {
    bearing = i2c_reg_read(0x60, 1);
    printf("Bearing from compass: %i\n", bearing);
    usleep(50 * 1000);
  }

  i2c_uninit();
  return 0;
}

