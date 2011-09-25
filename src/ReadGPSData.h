/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __READGPSDATA_H
#define __READGPSDATA_H

#include <vehicle.h>

void InitGPSSensor();
void ReadGPSData(vehicle::SensorData &data);

#endif // __READGPSDATA_H
