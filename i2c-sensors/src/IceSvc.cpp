#include "IceSvc.h"


// 100ms, 10Hz
#define UPDATE_INTERVAL (100 * 1000)

static const sensors::SensorDescription sonars_descr[] = 
{
  {0xE2, sensors::Range, 0, 2 * 100, 1000000.0 / UPDATE_INTERVAL, 
   "Left sonar", "srf02"},
  {0xE4, sensors::Range, 0, 2 * 100, 1000000.0 / UPDATE_INTERVAL, 
   "Right sonar", "srf02"},
  {0xE0, sensors::Range, 0, 2 * 100, 1000000.0 / UPDATE_INTERVAL, 
   "Front sonar", "srf08"},
  {0xE6, sensors::Range, 0, 2 * 100, 1000000.0 / UPDATE_INTERVAL, 
   "Back sonar", "srf02"}
 };

static const sensors::SensorDescription compas_descr[] = 
{
  {0xC0, sensors::Compass, 0, 359.9, 1000000.0 / UPDATE_INTERVAL, 
   "Compass", "cmps03"}
};


IceSvc::IceSvc()
{
}


bool 
IceSvc::start(int argc, char *argv[], int &status)
{
  Ice::ObjectAdapterPtr adapter = 
    this->communicator()->createObjectAdapter("i2c-sensors");

  this->sadm = new SonarAdmin();
  Ice::ObjectPrx obj = 
    adapter->add(sadm, 
		 this->communicator()->stringToIdentity("sonars-admin"));
  admin::StatePrx state_prx = admin::StatePrx::uncheckedCast(obj);
  sensors::SensorDescriptionSeq sds(sonars_descr, sonars_descr + sizeof(sonars_descr) / sizeof(sonars_descr[0]));
  this->sonar_group = new SonarGroup(state_prx, sds, this->bus_mutex);
  sadm->setSensorGroup(this->sonar_group);
  adapter->add(this->sonar_group, 
	       this->communicator()->stringToIdentity("sonars"));

  this->cadm = new CompassAdmin();
  obj = 
    adapter->add(cadm, 
		 this->communicator()->stringToIdentity("compass-admin"));
  state_prx = admin::StatePrx::uncheckedCast(obj);
  sensors::SensorDescriptionSeq cds(compas_descr, compas_descr + sizeof(compas_descr) / sizeof(compas_descr[0]));
  this->compass_group = new CompassGroup(state_prx, cds, this->bus_mutex);
  cadm->setSensorGroup(this->compass_group);
  adapter->add(this->compass_group, 
	       this->communicator()->stringToIdentity("compass"));

  adapter->activate();

  status = EXIT_SUCCESS;
  return true;
}


bool 
IceSvc::stop()
{
  this->sonar_group->requestStop();
  this->compass_group->requestStop();
  return Ice::Service::stop();
}
