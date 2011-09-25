/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __READSONARDATA_H
#define __READSONARDATA_H

#include <vehicle.h>

// 0x70 is 0xE0 >> 1 because of 7 bit address types used by I2C
#define US1ADDRESS 0x70

void InitSonarSensor();
void ReadSonarData(vehicle::SensorData &data);

#endif // __READSONARDATA_H
