#pragma once
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <vector>
#include <queue>
#include <atomic>

class ThreadPool
{
	boost::thread_group * m_threads;
	boost::mutex m_queueMutex;
	std::queue<boost::function<void()>> m_taskQueue;

	boost::condition_variable m_wakeUp;
	int m_numberOfThreads;

	void threadProc();
public:
	ThreadPool(); // use one thread per logical processor
	ThreadPool(int number_of_threads);
	void addTask(boost::function<void()> f);
	void join();
	~ThreadPool();
};

