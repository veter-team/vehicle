/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#include "UpdaterThread.h"
#include "from-u-boot/i2c_interface.h"


UpdaterThreadBase::UpdaterThreadBase(sensors::SensorFrameReceiverPrx cb,
				     IceUtil::Mutex &datamutex,
				     IceUtil::Mutex &busmutex,
				     sensors::SensorFrame &sf,
				     const addrlist_t &addresslist,
				     useconds_t updateinterval)
  : sensor_cb(cb), data_mutex(datamutex), bus_mutex(busmutex), 
    sensor_frame(sf), address_list(addresslist), 
    update_interval(updateinterval)
{
}


void 
UpdaterThreadBase::requestStop()
{
  this->should_stop = true;
}


void 
UpdaterThreadBase::run()
{
  this->should_stop = false;
  while(!this->should_stop)
    {
      try
	{
	  if(this->sensor_cb)
	    {
	      this->querySensors();
	      if(!this->sensor_frame.empty())
		  this->sensor_cb->nextSensorFrame(this->sensor_frame);
	      usleep(this->update_interval);
	    }
	  else
	    this->should_stop = true;
	}
      catch(const Ice::Exception& ex)
	{
	  //cout << ex << '\n';
	  //cout << "Forgetting sensor callback receiver\n";
	  this->sensor_cb = 0;
	  this->should_stop = true;
	}
    }
}
 

SonarUpdaterThread::SonarUpdaterThread(sensors::SensorFrameReceiverPrx cb,
				       IceUtil::Mutex &datamutex,
				       IceUtil::Mutex &busmutex,
				       sensors::SensorFrame &sf,
				       const addrlist_t &addresslist)
  : UpdaterThreadBase(cb, datamutex, busmutex, sf, addresslist, 0)
{
}


void  
SonarUpdaterThread::querySensors()
{
  { // To release bus mutex asap and let other activities on the bus
    // while waiting for ranging results
  IceUtil::Mutex::Lock lock(this->bus_mutex);
  // Query sonars
  for(addrlist_t::const_iterator addr = this->address_list.begin();
      addr != this->address_list.end(); ++addr)
    {
      i2c_reg_write(*addr, 0, 0x51); // start ranging
    }
  }

  // Wait for ranging to complete. Should wait at least 65ms
  // We will wait 100ms
  usleep((100 * 1000));
  
  IceUtil::Mutex::Lock dlock(this->data_mutex);
  IceUtil::Mutex::Lock block(this->bus_mutex);

  uint16_t range;
  sensors::SensorFrame::iterator sensor_data = this->sensor_frame.begin();
  for(addrlist_t::const_iterator addr = this->address_list.begin();
      addr != this->address_list.end(); ++addr)
    {
      range = i2c_reg_read(*addr, 2); // Range Hi byte
      range <<= 8;
      range += i2c_reg_read(*addr, 3); // Range Lo byte
      if(sensor_data->shortdata.empty())
	sensor_data->shortdata.push_back(range);
      else
	(*(sensor_data->shortdata.begin())) = range;
      ++sensor_data;
    }
}


CompassUpdaterThread::CompassUpdaterThread(sensors::SensorFrameReceiverPrx cb,
					   IceUtil::Mutex &datamutex,
					   IceUtil::Mutex &busmutex,
					   sensors::SensorFrame &sf,
					   const addrlist_t &addresslist)
  : UpdaterThreadBase(cb, datamutex, busmutex, sf, addresslist, 200 * 1000)
{
}


void 
CompassUpdaterThread::querySensors()
{
  IceUtil::Mutex::Lock dlock(this->data_mutex);
  IceUtil::Mutex::Lock block(this->bus_mutex);

  // Read compass
  uint8_t hi = 0; // Bearing Hi byte
  uint8_t lo = 0; // Bearing Lo byte
  float bearing = 0;
  sensors::SensorFrame::iterator sensor_data = this->sensor_frame.begin();
  for(addrlist_t::const_iterator addr = this->address_list.begin();
      addr != this->address_list.end(); ++addr)
    {
      // Bearing
      hi = i2c_reg_read(*addr, 2); // Bearing Hi byte
      lo = i2c_reg_read(*addr, 3); // Bearing Lo byte
      bearing = ((float)((hi << 8)+lo)) / 10.0;
      if(sensor_data->floatdata.empty())
	sensor_data->floatdata.push_back(bearing);
      else
	(*(sensor_data->floatdata.begin())) = bearing;
      /*
      // Pitch
      pitch = i2c_reg_read(s->id, 4);
      if(sensor_data->bytedata.empty())
	sensor_data->bytedata.push_back(pitch);
      else
	(*(sensor_data->bytedata.begin())) = pitch;
      ++sensor_data;

      // Roll
      roll = i2c_reg_read(s->id, 5);
      if(sensor_data->bytedata.empty())
	sensor_data->bytedata.push_back(roll);
      else
	(*(sensor_data->bytedata.begin())) = roll;
      */
      ++sensor_data;
    }
}
