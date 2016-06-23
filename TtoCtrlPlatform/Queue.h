#ifndef _QUEUE_H_
#define _QUEUE_H_

#pragma warning(disable:4786)

#include <deque>
#include "Lock.h"

template<typename T>
class Queue
{
private:
	std::deque<T> Q_;
	Mutex mutex_;
	const Queue<T> & operator = ( const Queue<T>& );
	Queue<T> ( const Queue<T>& );
public:
	Queue<T>(){}
	void put(const T& t){
		Guard<Mutex> t_lock(mutex_);
		Q_.push_back(t);
	}
	T get(){
		T t;
		try{
			Guard<Mutex> t_lock(mutex_);
			t = Q_.front();
			Q_.pop_front();
		}catch(...)
		{
			throw(exception("Queue.Empty"));
		}		
		return t;
	}
	int qsize() const throw(){
		return Q_.size();
	}
	void clear() throw(){
		Guard<Mutex> t_lock(mutex_);
		Q_.clear();
	}
};
#endif