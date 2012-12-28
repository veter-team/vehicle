#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "from-u-boot/i2c_interface.h"
#include "from-u-boot/enable_i2c_clocks.h"


int 
main(int argc, char **argv)
{
  size_t i;
  int version;
  int res = enable_i2c_clocls();
  uint8_t hi, lo;
  uint16_t range = 0;
  uint8_t SONAR = 0xE0;

  if(res)
    {
      printf("Error enabling I2C clocks: %i\n", res);
      return res;
    }

  if(argc == 2)
    SONAR = strtol(argv[1], NULL, 16);

  printf("Using address 0x%X (0x%X).\nYou can provide address as a parameter (in hexadecimal without 0x)\n", SONAR, SONAR >> 1);
  SONAR >>= 1;

  i2c_init(100000, 1);
  printf("I2C-2 initialized\n");
  usleep(100 * 1000);
  version = i2c_reg_read(SONAR, 0);
  printf("Sonar firmware version: %i\n", version);
 
  // Adjust gain (31 is maximum and power-on default)
  i2c_reg_write(SONAR, 1, 7);

  for(i = 0; i < 200; ++i)
  {
    i2c_reg_write(SONAR, 0, 0x51); // start ranging

    // Wait for ranging to complete 65ms
    usleep(65 * 1000);
    hi = i2c_reg_read(SONAR, 2); // Range Hi byte
    lo = i2c_reg_read(SONAR, 3); // Range Lo byte
    range = (hi << 8) + lo;
    printf("Range: %u (%u %u)\n", range, hi, lo);
    //printf("Range: %u cm\n", range);

    usleep(100 * 1000);
  }

  i2c_uninit();
  return 0;
}

