/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "NavSensorThread.h"
#include <unistd.h>
#include <fcntl.h>
#include "ReadSonarData.h"
#include "ReadCompassData.h"
#include "ReadGPSData.h"
#include "i2c-from-u-boot/i2c_interface.h"



NavSensorThread::NavSensorThread()
: shutdown_requested(false)
{
  // Initialize i2c bus 2 with 100Kb/s and slave address 1
  // Slave address does not matter since we are acting as master.
  i2c_init(100000, 1);
}


NavSensorThread::~NavSensorThread()
{
  i2c_uninit();
}


void 
NavSensorThread::run()
{
  printf("Sensor data ackquisition thread started\n");

  InitGPSSensor();
  InitCompassSensor();
  //InitSonarSensor();

  this->shutdown_requested = false;

  while(!this->shutdown_requested)
  {
    usleep(50 * 1000); // Sleep 50ms for 20Hz
    SyncT::Lock lock(this->sync);

    for(vehicle::SensorFrame::iterator i = this->sensor_data.begin(); 
        i != this->sensor_data.end(); ++i)
    {
      switch(i->sensorid)
      {
        case SONAR:
        //ReadSonarData(*i);
        break;

        case COMPASS:
        ReadCompassData(*i);
        break;

        case GPS:
        ReadGPSData(*i);
        break;

        default:
        // Cameras are specially handled in the VideoCameraSensorI
        break;
      }
    }

    //printf("See if we have something to send\n");

    if(!this->sensor_data.empty() && this->sensor_callback)
    {
      try
      {
        //printf("Sending %i sensor data elements\n", this->sensor_data.size());
        this->sensor_callback->nextSensorFrame(this->sensor_data);
        //printf("Sent\n", this->sensor_data.size());
      }
      catch(const Ice::Exception& ex)
      {
        printf("Catch Ice exception:\n%s\n", ex.toString().c_str());
        this->sensor_callback = NULL;
        printf("Sensor callback is set to NULL\n");
      }
      // Each data element is an array and should be cleared
      // after sending.
      for(vehicle::SensorFrame::iterator i = this->sensor_data.begin(); 
          i != this->sensor_data.end(); ++i)
      {
        i->bytedata.clear();
        i->shortdata.clear();
        i->intdata.clear();
        i->longdata.clear();
        i->floatdata.clear();
      }
    }
  }
  printf("NavSensorThread::run() exited\n");
  this->_started = false;
}


void 
NavSensorThread::requestShutdown()
{
  this->shutdown_requested = true;
}


void 
NavSensorThread::watchSensor(SensorID sensorid)
{
  SyncT::Lock lock(this->sync);

  // See if we already watching this sensor
  for(vehicle::SensorFrame::const_iterator i = this->sensor_data.begin(); 
      i != this->sensor_data.end(); ++i)
    if(i->sensorid == sensorid)
      return;

  // If we are here then we were not watching this sensor. Add it.
  vehicle::SensorData d;
  d.sensorid = sensorid;
  this->sensor_data.push_back(d);
}


void 
NavSensorThread::unwatchSensor(SensorID sensorid)
{
  SyncT::Lock lock(this->sync);

  for(vehicle::SensorFrame::iterator i = this->sensor_data.begin(); 
      i != this->sensor_data.end(); ++i)
    if(i->sensorid == sensorid)
    {
      this->sensor_data.erase(i);
      return;
    }
  printf("--- NavSensorThread unwatching sensor id %i\n", sensorid);
}


void 
NavSensorThread::setCallback(const vehicle::SensorFrameReceiverPrx &callback)
{
  SyncT::Lock lock(this->sync);
  this->sensor_callback = vehicle::SensorFrameReceiverPrx::uncheckedCast(callback->ice_oneway());
}

