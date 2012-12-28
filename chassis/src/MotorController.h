#ifndef __MOTORCONTROLLER_H
#define __MOTORCONTROLLER_H

#include <iostream>
#include <stdexcept>
#include <rtdm/rtdm.h>
#include <native/task.h>
#include "Queue.h"
#include "interface.h"
#include "RangeMap.h"

using namespace std;


template <typename ActuatorFrame> 
class MotorController
{
 public:
  MotorController(const char *dev_name) 
    : input_queue(60*60*100), 
    output_queue(60*10), 
    rt_dev_name(dev_name),
    device(-1)
    {
    }

  ~MotorController() { this->uninit(); }

 public:
  void init();
  void uninit();
  void setActuators(const ActuatorFrame &duties, bool should_wait);

 private:
  typedef pair<ActuatorFrame, bool> request_t;
  typedef Queue<request_t> InputQueue;
  typedef Queue<ActuatorFrame> OutputQueue;

  static void rt_motor_task(void *arg);

  InputQueue input_queue;
  OutputQueue output_queue;
  const string rt_dev_name;
  int device;
  RT_TASK motor_task;
};


template <typename ActuatorFrame>
void 
MotorController<ActuatorFrame>::init()
{
  if(this->device >= 0)
    return;

  // open the device
  this->device = rt_dev_open(DEVICE_NAME, 0);
  if(this->device < 0)
    throw runtime_error(string("can't open device " + this->rt_dev_name));

  int ret = rt_task_create(&(this->motor_task), "motor", 0, 99, 0);
  if(0 != ret)
    {
      string error = "rt_task_create() failed.\n";
      switch(ret)
	{
	case -ENOMEM: 
	  error += "ENOMEM: the system fails to get enough dynamic memory from "
	    "the global real-time heap in order to create or register "
	    "the task.";

	case -EEXIST: 
	  error += "EEXIST: the name is already in use by some "
	    "registered object.";

	case -EPERM: 
	  error += "EPERM: this service was called from an "
	    "asynchronous context.";

	default:
	  error += "Unknown error code";
	}
      throw runtime_error(error);
    }

  rt_task_start(&(this->motor_task), rt_motor_task, (void*)this);
}


template <typename ActuatorFrame>
void 
MotorController<ActuatorFrame>::uninit()
{
  if(this->device < 0)
    return;

  rt_task_delete(&(this->motor_task));

  // close the device
  int ret = rt_dev_close(this->device);
  this->device = -1;
  if(ret < 0)
    throw runtime_error(string("can't close device " + this->rt_dev_name));
}


template <typename ActuatorFrame>
void 
MotorController<ActuatorFrame>::setActuators(const ActuatorFrame &duties,
					     bool should_wait)
{
  /*
  for(typename ActuatorFrame::const_iterator i = duties.begin(); i != duties.end(); ++i)
    {
      cout << "Setting actuator #" << i->id
	   << " -> speed: " << i->speed 
	   << " distance: " << i->distance
	   << endl;
    }
  cout << endl;
  */
  this->input_queue.put(make_pair(duties, should_wait));
  if(should_wait)
    this->output_queue.get();
}


template <typename ActuatorFrame>
void 
MotorController<ActuatorFrame>::rt_motor_task(void *arg)
{
  MotorController<ActuatorFrame> *controller = 
    (MotorController<ActuatorFrame>*)arg;
  cout << "RT task thread started\n";

  for(;;)
    {
      request_t request = controller->input_queue.get();
      ActuatorFrame &frm = request.first;

      ControlCommandRequest cmd[frm.size()];
      size_t n = 0;
      for(typename ActuatorFrame::const_iterator f = frm.begin(); f != frm.end(); ++f)
	{
	  cmd[n].motor = f->id;
	  if(f->speed >= 0)
	    {
	      cmd[n].direction = 0;
	      cmd[n].target_speed = f->speed;
	    }
	  else
	    {
	      cmd[n].direction = 1;
	      cmd[n].target_speed = -(f->speed);
	    }

	  /* If target_speed is 0 but we still call
	  SPEED_TO_STATES(...), then, probably because of rounding, we
	  will get 1 state/dt request instead of required 0. It will
	  cause the wheels to always rotate very slow even if we
	  request stop with 0% */
	  if(cmd[n].target_speed != 0)
	    cmd[n].target_speed = 
	      uint16_t(SPEED_TO_STATES(range_map(0.0f, 100.0f, 
						 MIN_SPEED, MAX_SPEED, 
						 float(cmd[n].target_speed))));
	  cmd[n].states_to_go = 1000.0 / 3.0 * f->distance;
	  ++n;
	}

      size_t size = rt_dev_write(controller->device, 
				 (const void*)cmd, sizeof(cmd));

      if(request.second) // if we should wait for commands completion
	{
	  ControlCommandResponse read;
	  for(typename ActuatorFrame::iterator f = frm.begin(); f != frm.end(); ++f)
	    {
	      if(f->speed != 0)
		{
		  size = rt_dev_read(controller->device, (void*)&read, sizeof(read));
		  f->id = read.motor;
		  f->speed = 0;
		  f->distance = read.result;
		}
	    }
	  controller->output_queue.put(frm);
	}
    }
  cout << "RT task thread exited\n";
}

#endif // __MOTORCONTROLLER_H
