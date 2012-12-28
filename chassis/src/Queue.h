/* Copyright (c) 2010 Andrey Nechypurenko
   See the file LICENSE for copying permission. 
*/

#ifndef __QUEUE_H
#define __QUEUE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <list>
#include <stdexcept>


template<class T> 
class Queue : public IceUtil::Monitor<IceUtil::Mutex>
{
 public:
   Queue(IceUtil::Int64 wait_timeout_seconds = 10) 
     : timeout(IceUtil::Time::seconds(wait_timeout_seconds)) {}

 public:
  void put(const T& item)
  {
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    this->q.push_back(item);
    if(this->q.size() == 1)
      this->notify();
  }

  T get()
  {
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    while(this->q.size() == 0)
      //this->wait();
      if(false == this->timedWait(this->timeout))
      {
        throw std::runtime_error("timeout in Queue::get()");
      }
    T item = this->q.front();
    this->q.pop_front();
    return item;
  }

  typename std::list<T>::size_type size() const 
    {
      IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
      return this->q.size();
    }

 private:
  const IceUtil::Time timeout;
  std::list<T> q;
};

#endif // __QUEUE_H 
