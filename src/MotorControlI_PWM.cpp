/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "MotorControlI_PWM.h"
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>

#ifndef WIN32
#include "omap3530-pwm.h"
#endif

static const short delta = 5;

/* maps integer x in the range [a..b] to [c..d] */
template<typename sourceT, typename targetT>
targetT 
range_map(sourceT a, sourceT b, targetT c, targetT d, sourceT x)
{
  if (a == b) return 0;

  return c + (d - c)*((targetT)(x - a))/(b - a);
}


MotorControlI::MotorControlI(const Ice::PropertiesPtr &properties)
  : mem_fd(-1), motor_left_gpio(10), motor_right_gpio(11),
    motor_left_range_lo(0.05f),
    motor_left_range_hi(0.1f),
    motor_right_range_lo(0.05f),
    motor_right_range_hi(0.1f),
    prev_left((motor_left_range_hi - motor_left_range_lo) / 2.0f),
    prev_right((motor_right_range_hi - motor_right_range_lo) / 2.0f),
    prev_steer(50),
    prev_accel(50)
{
  std::string prop = properties->getProperty("Actuators.motorl.params");
  std::istringstream is1(prop);
  is1 >> this->motor_left_gpio;
  is1 >> this->motor_left_range_lo;
  is1 >> this->motor_left_range_hi;

  prop = properties->getProperty("Actuators.motorr.params");
  std::istringstream is2(prop);
  is2 >> this->motor_right_gpio;
  is2 >> this->motor_right_range_lo;
  is2 >> this->motor_right_range_hi;

#ifndef WIN32
  this->mem_fd = pwm_open_devmem();
#endif
  if(this->mem_fd == -1)
    {
      fprintf(stderr, "Unable to open /dev/mem, are you root?\n");
    }
#ifndef WIN32
  else
    {
      // Set instances 10 and 11 to use the 13 Mhz clock
      pwm_config_clock(this->mem_fd, TRUE, TRUE);
      this->gpt_left = pwm_mmap_instance(mem_fd, this->motor_left_gpio);
      this->gpt_right = pwm_mmap_instance(mem_fd, this->motor_right_gpio);

      // Get the resolution for 20 kHz PWM
      this->resolution = pwm_calc_resolution(50, PWM_FREQUENCY_13MHZ);
    }
#endif
}


MotorControlI::~MotorControlI()
{
#ifndef WIN32
  if(this->mem_fd != -1)
    {
      pwm_munmap_instance(this->gpt_left);
      pwm_munmap_instance(this->gpt_right);
      pwm_close_devmem(this->mem_fd);
    }
#endif
}


void
MotorControlI::setDuties(const vehicle::ActuatorFrame& duties)
{
  float duty_left;
  float duty_right;

  for(vehicle::ActuatorFrame::const_iterator d = duties.begin();
        d != duties.end(); ++d)
    {
      if(d->id == 0)
        prev_steer = d->duty;
      else if(d->id == 1)
        prev_accel = d->duty;
    }

  if(abs(prev_accel - 50) <= delta)
    { // rotate on place
      duty_right = range_map(short(0), short(100), 
                             this->motor_right_range_lo, 
                             this->motor_right_range_hi, 
                             prev_steer);
      duty_left = range_map(short(0), short(100), 
                            this->motor_left_range_lo, 
                            this->motor_left_range_hi, 
                            short(100 - prev_steer));
    }
  else
    {// break one side
      if(prev_steer < 50)
        {// break right side
          duty_right = range_map(short(0), short(100), 
                                 this->motor_right_range_lo, 
                                 this->motor_right_range_hi, 
                                 short(prev_accel - range_map(short(0), 
                                                              short(49), 
                                                              prev_accel, 
                                                              short(0),
                                                              prev_steer)));
          duty_left = range_map(short(0), short(100), 
                                this->motor_left_range_lo, 
                                this->motor_left_range_hi, 
                                prev_accel);
        }
      else
        {// break left side
          duty_left = range_map(short(0), short(100), 
                                this->motor_left_range_lo, 
                                this->motor_left_range_hi, 
                                short(prev_accel - range_map(short(50), 
                                                             short(100), 
                                                             short(0),
                                                             prev_accel, 
                                                             prev_steer)));
          duty_right = range_map(short(0), short(100), 
                                 this->motor_right_range_lo, 
                                 this->motor_right_range_hi, 
                                 prev_accel);
        }
    }

  if(prev_left != duty_left || prev_right != duty_right)
    {
      if(prev_left != duty_left)
        {
          if(this->mem_fd == -1)
            printf("left motor duty set to: %f\n", duty_left);
#ifndef WIN32
          else
            pwm_config_timer(this->gpt_left, this->resolution, duty_left);
#endif
          prev_left = duty_left;
        }

      if(prev_right != duty_right)
        {
          if(this->mem_fd == -1)
            printf("right motor duty set to: %f\n", duty_right);
#ifndef WIN32
          else
            pwm_config_timer(this->gpt_right, this->resolution, duty_right);
#endif
          prev_right = duty_right;
        }

#ifndef WIN32
      usleep(5 * 1000); // let the controller react to the changes
#else
      Sleep(5);
#endif
    }
}
