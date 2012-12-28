#ifndef __PIDPARAMS_H
#define __PIDPARAMS_H


/**
 * Structure used to configure PID controller as well as to keep
 * run-time values required for controller output calculation.
 */
typedef struct tagPIDParams
{
  int max; // Maximum control output value
  int min; // Minimum control output value
  int K[3]; // P, I and D coefficients * 1000
  int prev_error; // Previous error (for differential calculation)
  int integral; // Accumulated error (for integral calculation)
} pidparams_t;


#endif // __PIDPARAMS_H
