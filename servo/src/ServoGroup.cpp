#include <stdexcept>
#include <string>
#include <Ice/Service.h>
#include "ServoGroup.h"
#include "range_map.h"


using namespace std;

static const float motor_range_lo = 0.06f;
static const float motor_range_hi = 0.11f;

ServoGroup::ServoGroup(admin::StatePrx &prx)
  : state_prx(prx), is_started(false), mem_fd(-1), gpio_timer(NULL),
    current_duty(motor_range_lo + (motor_range_hi - motor_range_lo) / 2)
{
  // Position servo in the middle on start-up
  this->setStarted(true);
  usleep(500 * 1000); // Let servo rotate
  this->setStarted(false);
}


admin::StatePrx 
ServoGroup::getStateInterface(const Ice::Current&)
{
  return this->state_prx;
}


actuators::ActuatorDescriptionSeq 
ServoGroup::getActuatorDescription(const Ice::Current&)
{
  const actuators::ActuatorDescription desc = 
    {0, "Front camera servo", "futaba"};

  actuators::ActuatorDescriptionSeq ret;
  ret.push_back(desc);
  return ret;
}


void 
ServoGroup::setStarted(bool started)
{
  IceUtil::Mutex::Lock lock(this->hw_mutex);

  if(this->is_started == false && started == true)
    {// start
      static const size_t servo_gpio = 11;

      this->mem_fd = pwm_open_devmem();
      if(this->mem_fd == -1)
	{
	  Ice::LoggerPtr logger = 
	    Ice::Service::instance()->communicator()->getLogger();
	  string msg = "Unable to open /dev/mem, are you root?";
	  logger->error(msg);
	  throw runtime_error(msg);
	}

      // Enable ICLK clock
      pwm_iclken_clock(this->mem_fd, true, true);
      // Enable FCLK clock
      pwm_fclken_clock(this->mem_fd, true, true);

      // Set instances 10 and 11 to use the 13 Mhz clock
      pwm_config_clock(this->mem_fd, true, true);
      this->gpio_timer = pwm_mmap_instance(mem_fd, servo_gpio);
      // Get the resolution for 20ms period (50Hz) PWM
      this->resolution = pwm_calc_resolution(50, PWM_FREQUENCY_13MHZ);
      pwm_config_timer(this->gpio_timer, 
		       this->resolution, 
		       this->current_duty); // middle position
    }
  else if(this->is_started == true && started == false)
    {// stop
      pwm_munmap_instance(this->gpio_timer);
      // Disable ICLK clock
      pwm_iclken_clock(this->mem_fd, false, false);
      // Disable FCLK clock
      pwm_fclken_clock(this->mem_fd, false, false);
      pwm_close_devmem(this->mem_fd);
      this->mem_fd = -1;
    }
  
  this->is_started = started;
}


void 
ServoGroup::setActuatorsAndWait(const actuators::ActuatorFrame &duties, 
				 const Ice::Current &current)
{
  // Waiting is not supported yet
  this->setActuatorsNoWait(duties, current);
}


void 
ServoGroup::setActuatorsNoWait(const actuators::ActuatorFrame &duties, 
				 const Ice::Current &current)
{
  IceUtil::Mutex::Lock lock(this->hw_mutex);
  if(!this->is_started || duties.empty() || duties[0].id != 0)
    return;
  float cur_duty_01 = 
    range_map(motor_range_lo, motor_range_hi, 0.0f, 1.0f, this->current_duty);

  if(duties[0].speed >= 0)
    this->current_duty = 
      min(range_map(0.0f, 1.0f, motor_range_lo, motor_range_hi, 
		    cur_duty_01 + duties[0].distance),
	  motor_range_hi);
  else
    this->current_duty = 
      max(range_map(0.0f, 1.0f, motor_range_lo, motor_range_hi, 
		    cur_duty_01 - duties[0].distance),
	  motor_range_lo);

  //cout << "Set duty to [0.06:0.11]: " << this->current_duty << endl;
  pwm_config_timer(this->gpio_timer, 
		   this->resolution, 
		   this->current_duty);
}
