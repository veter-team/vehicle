/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "ServoControl.h"
#include "xenopwm.h"
#include <sstream>



ServoControl::ServoControl(const Ice::PropertiesPtr &properties,
                           const std::string &servo_name)
{
  int status;
  PwmDesc pwm[1];
  // Initialize with default values
  pwm[0].channel = 0;
  pwm[0].pwmMinWidth = 950;
  pwm[0].pwmMaxWidth = 2050;
  // Try to read from config file
  std::string prop = 
    properties->getProperty("Actuators." + servo_name + ".params");
  std::istringstream is(prop);
  is >> pwm[0].channel;
  is >> pwm[0].pwmMinWidth;
  is >> pwm[0].pwmMaxWidth;

  printf("Initializing Xenomai PWM generator\n");

  status = initpwm(pwm, sizeof(pwm) / sizeof(pwm[0]));
  if(status != 0)
    fprintf(stderr, "Error: %i was returned\n", status);
  else
    setpwmwidth(0, 50); // 50% duty is middle position

}


ServoControl::~ServoControl()
{
  cleanuppwm();
}


void 
ServoControl::setDuties(const vehicle::ActuatorData &data)
{
  setpwmwidth(0, data.duty);
}

