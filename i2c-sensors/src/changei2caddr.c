/**
 * Possible addresses: E0, E2, E4, E6, E8, EA, EC, EE, F0, 
 *                     F2, F4, F6, F8, FA, FC, FE
 * Factory default: E0
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "from-u-boot/i2c_interface.h"
#include "from-u-boot/enable_i2c_clocks.h"


int 
main(int argc, char **argv)
{
  size_t i;
  int version;
  int res = enable_i2c_clocls();
  uint8_t current_address;
  // 4-th element is a new address
  uint8_t to_send[4] = {0xA0, 0xAA, 0xA5, 0x00}; 

  if(argc != 3)
  {
    printf("Usage: changei2caddr <old_addr_in_hex> <new_addr_in_hex>\n");
    return 1;
  }

  if(res)
    {
      printf("Error enabling I2C clocks: %i\n", res);
      return res;
    }

  i2c_init(100000, 1);

  usleep(100 * 1000);
  current_address = strtol(argv[1], NULL, 16);
  current_address >>= 1; // Translate to 7-bit format
  to_send[3] = strtol(argv[2], NULL, 16);
  printf("Changing 0x%X to 0x%X. Press enter to confirm.\n", 
         current_address, to_send[3]);
  getchar();


  usleep(100 * 1000);
  version = i2c_reg_read(current_address, 0);
  printf("Firmware version read from old address (%X): %i\n", 
         current_address, version);

  usleep(100 * 1000);
  for(i = 0; i < sizeof(to_send)/sizeof(to_send[0]); ++i)
    i2c_reg_write(current_address, 0, to_send[i]);
  
  usleep(100 * 1000);
  version = i2c_reg_read(to_send[3] >> 1, 0);
  printf("Firmware version read from new address (%X): %i\n",
         to_send[3], version);

  i2c_uninit();
  return 0;
}

