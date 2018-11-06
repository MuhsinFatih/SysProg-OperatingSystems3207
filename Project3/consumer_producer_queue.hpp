#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>


template<typename T>
class ConsumerProducerQueue
{
	std::condition_variable cond;
	std::mutex mutex;
	std::queue<T> queue;
	int maxsize; // 0 means unlimited
	
public:
	ConsumerProducerQueue(int mxsz) : maxsize(mxsz) {}
	
	void add(T request) {
		std::unique_lock<std::mutex> lock(mutex);
		cond.wait(lock, [this]{return !maxsize ||( queue.size() < maxsize);}); // wait until there is slot available
		queue.push(request);
		lock.unlock();
		cond.notify_all();
		// printf("added, %i\n", queue.size());
	}

	T consume() {
		std::unique_lock<std::mutex> lock(mutex);
		// cond.wait(lock, queue.size() == size); // wait until queue is full
		cond.wait(lock, [this]{return queue.size() != 0;}); // wait until there is work to consume
		T request = queue.front();
		queue.pop();
		lock.unlock();
		cond.notify_all();
		return request;
	}

	size_t length() const {
		return queue.size();
	}
};