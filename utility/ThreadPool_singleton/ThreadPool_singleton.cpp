#include "ThreadPool_singleton.h"
#include <iostream>
using namespace ix::thread;
using namespace std;

ThreadPool & ThreadPool::Instance(size_t thread_count)
{
	static ThreadPool instance(thread_count);
	return instance;
}

ThreadPool::ThreadPool(size_t thread_count)
{
	for (size_t i = 0; i < thread_count; i++) {
		workers.emplace_back(&ThreadPool::worker_loop,this);
	}
}

void ThreadPool::shut_down()
{
	{
		lock_guard<mutex> lock(queue_mutex);
		stop.store(true);
	}
	condition.notify_all();

	for (std::thread &t : workers) {
		if (t.joinable()) {
			t.join();
		}
	}
}

ThreadPool::~ThreadPool()
{

}

void ix::thread::ThreadPool::worker_loop()
{
	while (!stop.load()) {
		//cout << "线程["<< this_thread::get_id() << "] 进入等待模式"<< endl;
		std::function<void()> task;
		{
			unique_lock<mutex> lock(queue_mutex);
			condition.wait(lock, [this]() {return stop.load() || !tasks.empty();});
			
			if (stop.load() && tasks.empty()) {
				return;
			}
			task = std::move(tasks.front());
			tasks.pop();
		}
		//cout << "线程["<< this_thread::get_id() << "] 被唤醒"<< endl;
		task();
	}
}
