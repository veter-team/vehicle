/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __STATYSTIC_H
#define __STATYSTIC_H

#include <sys/types.h>
#ifdef WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif


class Statystic
{
 public:
  Statystic();
  ~Statystic();

 public:

  void frameQueued(size_t frame_size);
  void frameSent(size_t frame_size);

  int getQoSState() const {return qos_state;}

 private:
  size_t avg_buf_size;
  size_t buf_size;
  int qos_state;
  time_t last_change;

  void updateQoSState(size_t frame_size);
};


#endif // __STATYSTIC_H
