/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __READCOMPASSDATA_H
#define __READCOMPASSDATA_H

#include <vehicle.h>

// 0x60 is 0xC0 >> 1
#define CMPS03ADDRESS 0x60

void InitCompassSensor();
void ReadCompassData(vehicle::SensorData &data);

#endif // __READCOMPASSDATA_H
