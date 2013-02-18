/* Copyright (c) 2013 Andrey Nechypurenko
   See the file COPYING for copying permission.
*/
#ifndef __INTERFACE_H
#define __INTERFACE_H

#include <rtdm/rtdm.h>
#include "pid-params.h"


#define DEVICE_NAME "tb6612fng-ctl"

// In m/s
#define MIN_SPEED 0.02f
#define MAX_SPEED 0.4f

// In milliseconds
#define DELTA_T 50

#define STATES_PER_ROTATION 333.0f
// Wheel diameter is 6cm = 0.06m.
#define WHEEL_D 0.06f
// 1 RPS will lead to speed Pi*0.06 ~ 0.1885 m/s
#define ONE_RPS_SPEED (3.14159f * WHEEL_D)

#define SPEED_TO_STATES(speed) (speed / ONE_RPS_SPEED * STATES_PER_ROTATION / 1000 * DELTA_T)

#define STATES_TO_SPPED(states) (1000.0f/DELTA_T*states * ONE_RPS_SPEED/STATES_PER_ROTATION)


typedef struct tagControlCommandRequest
{
  unsigned char motor; // 0 - A, 1 - B
  unsigned char direction; // 0 - forward, 1 - backward
  uint16_t target_speed; // target speed in state changes within DELTA_T

  // We have 1000 state changes per 3 revolutions
  // Current wheel diameter is 0.06m
  uint32_t states_to_go;
} ControlCommandRequest;

typedef struct tagControlCommandResponse
{
  uint32_t result; // 0 - target reached; > 0 ramaining states to go
  uint8_t motor;
} ControlCommandResponse;


/**
 * PID controller configuration (for ioctl)
 */
typedef struct pid_config
{
  int max; // Maximum control output value
  int min; // Minimum control output value
  int Kp, Ki, Kd; // P, I and D coefficients * 1000
} pid_config_t[2];


typedef struct motor_speed_info
{
  // PID crosstrack error since last ioctl call.
  long unsigned int crosstrack_error;
  nanosecs_rel_t delta_t; // observed time interval
  uint16_t state_changes_counter;
  uint8_t direction;
} motor_speed_info_t;
  
typedef motor_speed_info_t speed_info_t[2];
typedef pidparams_t pid_instance_params_t[2];

#define RTIOC_TYPE_MOTOR        RTDM_CLASS_EXPERIMENTAL
#define RTDM_SUBCLASS_TB6612FNG 2111


/**
 * Get PID controller configuration
 *
 * @param[out] arg Pointer to configuration buffer (struct pid_config)
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - Kernel module initialization/cleanup code
 * - Kernel-based task
 * - User-space task (RT, non-RT)
 *
 * Rescheduling: never.
 */
#define PID_GET_CONFIG	\
	_IOR(RTIOC_TYPE_MOTOR, 0x00, pid_config_t)

/**
 * Get current speed
 *
 * @param[out] arg Pointer to uint32_t variable to store current speed
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - Kernel module initialization/cleanup code
 * - Kernel-based task
 * - User-space task (RT, non-RT)
 *
 * Rescheduling: never.
 */
#define PID_GET_SPEED	\
	_IOR(RTIOC_TYPE_MOTOR, 0x01, speed_info_t)

/**
 * Set PID controller configuration
 *
 * @param[in] arg Pointer to configuration buffer (struct rtser_config)
 *
 * @return 0 on success, otherwise negative error code
 *
 * Environments:
 *
 * This service can be called from:
 *
 * - Kernel module initialization/cleanup code
 * - Kernel-based task
 * - User-space task (RT, non-RT)
 *
 * @note 
 *
 * Rescheduling: never.
 */
#define PID_SET_CONFIG	\
	_IOW(RTIOC_TYPE_MOTOR, 0x02, pid_config_t)

#endif // __INTERFACE_H
