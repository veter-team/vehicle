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


int 
main(int argc, char **argv)
{
  size_t i;
  int version;
  int res = enable_i2c_clocls();
  uint8_t hi, lo;
  uint8_t pitch, roll;
  float bearing = 0;
  uint8_t COMPAS = 0xC0;

  if(res)
    {
      printf("Error enabling I2C clocks: %i\n", res);
      return res;
    }

  if(argc == 2)
    COMPAS = strtol(argv[1], NULL, 16);
  else
    COMPAS = 0xC0;

  printf("Using address 0x%X (0x%X).\nYou can provide address as a parameter (in hexadecimal without 0x)\n", COMPAS, COMPAS >> 1);
  COMPAS >>= 1;

  i2c_init(100000, 1);
  printf("I2C-2 initialized\n");
  usleep(100 * 1000);
  version = i2c_reg_read(COMPAS, 0);
  printf("Compas firmware version: %i\n", version);
  
  for(i = 0; i < 200; ++i)
  {
    /*   
    Wire.beginTransmission(ADDRESS);           //starts communication with CMPS10
    Wire.write(2);                              //Sends the register we wish to start reading from
    Wire.endTransmission();

    Wire.requestFrom(ADDRESS, 4);              // Request 4 bytes from CMPS10
    while(Wire.available() < 4);               // Wait for bytes to become available
    highByte = Wire.read();           
    lowByte = Wire.read();            
    pitch = Wire.read();              
    roll = Wire.read();               
   
    bearing = ((highByte<<8)+lowByte)/10;      // Calculate full bearing
    fine = ((highByte<<8)+lowByte)%10;         // Calculate decimal place of bearing
   
    display_data(bearing, fine, pitch, roll);  // Display data to the LCD03
   
    delay(100);
    */


    usleep(100 * 1000);
    hi = i2c_reg_read(COMPAS, 2); // Bearing Hi byte
    lo = i2c_reg_read(COMPAS, 3); // Bearing Lo byte
    pitch = i2c_reg_read(COMPAS, 4); // Pitch
    roll = i2c_reg_read(COMPAS, 5); // Roll
    bearing = ((float)((hi << 8)+lo)) / 10.0;
    //printf("Range: %u (%u %u)\n", range, hi, lo);
    printf("Bearing: %f, pitch %u, roll %u\n", bearing, pitch, roll);

    usleep(200 * 1000);
  }

  i2c_uninit();
  return 0;
}

