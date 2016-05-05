#include <mutex>
#include <condition_variable>
#include <queue>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>

using namespace std;

#include "constants.h"

struct arg_struct {
    	char arg1_ip[IP_BUFSIZE];
   	int arg2_port;
};


template <typename T>
class BlockingQueue
{
 private:
  queue<T> queue_;
  mutex mutex_;
  condition_variable cond_full;
  condition_variable cond_empty;
 
 public:


  T pop()
  { 
  	std::unique_lock<std::mutex> mlock (mutex_);
    while (queue_.empty())
    {
      cond_empty.wait(mlock);
    }
    auto message = queue_.front();
    queue_.pop();
    mlock.unlock();
	cond_full.notify_one();

    return message;
  }

  T front()
  {
	std::unique_lock<std::mutex> mlock (mutex_);
	while (queue_.empty())
	{
		cond_empty.wait(mlock);
	}
	auto message = queue_.front();
	mlock.unlock();

	return message;
  }

	bool empty()
	{
		return queue_.empty();
	}
  
	bool full() 
	{ 
		if (queue_.size() >= MAXIMUM_SIZE){
			return true;
		}
		else{
			return false;
		}
	} 

  void push (string message)
  {	
  	std::unique_lock<std::mutex> mlock (mutex_);
	while (full())
	{
		cond_full.wait(mlock);
	}
    
    queue_.push(message);
    mlock.unlock();
    cond_empty.notify_one();
  }
  
};
