#ifndef __SENSORGROUPI_H
#define __SENSORGROUPI_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Handle.h>

#include <sensors.h>

#include <unistd.h>
#include <iostream>
#include <stdint.h>
#include <sstream>
#include "from-u-boot/i2c_interface.h"

using namespace std;


template <typename UpdaterThread>
class SensorGroupI : public sensors::SensorGroup
{
 public:
  SensorGroupI(admin::StatePrx &prx, 
	       const sensors::SensorDescriptionSeq &known_sensors,
	       IceUtil::Mutex &busmutex);
  ~SensorGroupI();

 public:
  void requestStart(); // Starts updater thread
  void requestStop(); // Request updater thread to quit

 public:
  virtual admin::StatePrx getStateInterface(const Ice::Current& = Ice::Current());

  virtual sensors::SensorDescriptionSeq getSensorDescription(const Ice::Current& = Ice::Current());

  virtual sensors::SensorFrame getCurrentValues(const Ice::Current& = Ice::Current());

  virtual bool setSensorReceiver(const sensors::SensorFrameReceiverPrx& callback,
				 const Ice::Current& = ::Ice::Current());

  virtual void cleanSensorReceiver(const Ice::Current& = ::Ice::Current());

 private:
  admin::StatePrx state_prx;
  sensors::SensorFrameReceiverPrx sensor_cb;
  IceUtil::Mutex data_mutex;
  IceUtil::Mutex& bus_mutex;
  sensors::SensorFrame sensor_frame;
  sensors::SensorDescriptionSeq sensors_descr;
  addrlist_t address_list;
  typedef IceUtil::Handle<UpdaterThread> UpdaterThreadPtr;
  UpdaterThreadPtr updater_thread;
};


template <typename UpdaterThread>
SensorGroupI<UpdaterThread>::SensorGroupI(admin::StatePrx &prx, 
			   const sensors::SensorDescriptionSeq &known_sensors,
			   IceUtil::Mutex &busmutex)
  : state_prx(prx), bus_mutex(busmutex)
{
  char buff[1024];
  uint8_t fwversion;
  Ice::LoggerPtr logger = 
    Ice::Service::instance()->communicator()->getLogger();
  for(sensors::SensorDescriptionSeq::const_iterator ks = known_sensors.begin();
      ks != known_sensors.end(); ++ks)
    {
      //fwversion = i2c_reg_read(sonars_descr[i].id >> 1, 0);
      if(i2c_read(ks->id >> 1, 0, 1, &fwversion, 1) != 0)
	{
	  ostringstream os;
	  os << "@0x" << hex << ks->id << dec << ": ";
	  os << ks->description 
	     << ", " << ks->vendorid 
	     << " NOT DETECTED" << ends;
	  logger->print(os.str());
	  continue;
	}
      /*
      if(ks->vendorid == "srf08")
	{// Adjust gain (31 is maximum and power-on default).
	 // 7 is experimentally found value which works well
	  const uint8_t gain = 7;
	  i2c_reg_write(ks->id >> 1, 1, gain);
	  ostringstream os;
	  os << "Gain adjusted to " << int(gain) << " for srf08\n" << ends;
	  logger->print(os.str());
	}
      */
      sensors::SensorDescription descr(*ks);
      descr.id = ks->id >> 1;
      sprintf(buff, "%s, firmware v%i", 
	      ks->vendorid.c_str(), fwversion);
      descr.vendorid = buff;
      this->sensors_descr.push_back(descr);
    }

  logger->print("Found I2C sensors:");
  for(sensors::SensorDescriptionSeq::const_iterator s = this->sensors_descr.begin(); s != this->sensors_descr.end(); ++s)
    {
      ostringstream os;
      os << "@0x" << hex << (s->id << 1) << dec << ": ";
      os << s->description << ", " << s->vendorid << ends;
      logger->print(os.str());

      sensors::SensorData d;
      d.sensorid = s->id << 1;
      this->sensor_frame.push_back(d);
      this->address_list.push_back(s->id);
    }
}


template <typename UpdaterThread>
SensorGroupI<UpdaterThread>::~SensorGroupI()
{
  if(this->updater_thread && this->updater_thread->isAlive())
    {
      this->updater_thread->requestStop();
      this->updater_thread->getThreadControl().join();
    }
}


template <typename UpdaterThread>
void 
SensorGroupI<UpdaterThread>::requestStop()
{
  if(this->updater_thread && this->updater_thread->isAlive())
    {
      this->updater_thread->requestStop();
      this->updater_thread->getThreadControl().join();
    }
}


template <typename UpdaterThread>
void 
SensorGroupI<UpdaterThread>::requestStart()
{
  if(this->updater_thread && this->updater_thread->isAlive())
    return;

  this->updater_thread = new UpdaterThread(this->sensor_cb,
					   this->data_mutex,
					   this->bus_mutex,
					   this->sensor_frame,
					   this->address_list);
  this->updater_thread->start();
}


template <typename UpdaterThread>
admin::StatePrx
SensorGroupI<UpdaterThread>::getStateInterface(const Ice::Current&)
{
  return this->state_prx;
}


template <typename UpdaterThread>
sensors::SensorDescriptionSeq
SensorGroupI<UpdaterThread>::getSensorDescription(const Ice::Current&)
{
  return this->sensors_descr;
}


template <typename UpdaterThread>
sensors::SensorFrame
SensorGroupI<UpdaterThread>::getCurrentValues(const Ice::Current&)
{
  this->updater_thread->querySensors();
  IceUtil::Mutex::Lock lock(data_mutex);
  return this->sensor_frame;
}


template <typename UpdaterThread>
bool
SensorGroupI<UpdaterThread>::setSensorReceiver(
  const sensors::SensorFrameReceiverPrx& callback,
  const Ice::Current&)
{
  this->sensor_cb = sensors::SensorFrameReceiverPrx::uncheckedCast(callback->ice_oneway()->ice_timeout(2000));;
  return true;
}


template <typename UpdaterThread>
void
SensorGroupI<UpdaterThread>::cleanSensorReceiver(const Ice::Current&)
{
  this->sensor_cb = 0;
}


#endif // __SENSORGROUPI_H
