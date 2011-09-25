/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#include "Statystic.h"
#include <stdio.h>
#include <time.h>

#define MIN_CHANGE_INTERVAL 30


struct WaterMark
{
  size_t low;
  size_t high;
  size_t sec_in_state;
};

static const WaterMark state_table[] = 
  {
    {1, 5, 0}, {1, 5, 0}, {1, 5, 0}
  };

static const size_t state_table_max_idx = sizeof(state_table) / sizeof(state_table[0]) - 1;

Statystic::Statystic()
  : avg_buf_size(0), buf_size(0), qos_state(0), last_change(time(NULL))
{
}


Statystic::~Statystic()
{
}


void 
Statystic::frameQueued(size_t frame_size)
{
  ++(this->buf_size);
  this->avg_buf_size = (this->avg_buf_size + this->buf_size) / 2;
  this->updateQoSState(0);
  //printf(" +++ Queued. Current buffer size: %u, average: %u\n", buf_size, avg_buf_size);
}


void 
Statystic::frameSent(size_t frame_size)
{
  if(this->buf_size)
    --(this->buf_size);
  this->avg_buf_size = (this->avg_buf_size + this->buf_size) / 2;
  this->updateQoSState(frame_size);
  //printf("--- Sent. Current buffer size: %u, average: %u\n", buf_size, avg_buf_size);
}


void 
Statystic::updateQoSState(size_t frame_size)
{
  if(this->qos_state > 0 
     && this->avg_buf_size < state_table[this->qos_state].low)
    {
      time_t now = time(NULL);
      if(now - this->last_change > MIN_CHANGE_INTERVAL)
        {
          this->last_change = now;
          --(this->qos_state);
        }
    }
  else if(this->qos_state < state_table_max_idx
          && this->avg_buf_size > state_table[this->qos_state].high)
    {
      time_t now = time(NULL);
      //if(now - this->last_change > MIN_CHANGE_INTERVAL)
        {
          this->last_change = now;
          ++(this->qos_state);
        }
    }

  //printf("Avg buff size: %u\n", this->avg_buf_size);
}
