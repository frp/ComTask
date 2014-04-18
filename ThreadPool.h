#pragma once
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <vector>
#include <queue>
#include <atomic>

class ThreadPool
{
	std::vector<boost::thread> m_threads;
	boost::mutex m_queueMutex;
	std::queue<boost::function<void()>> m_taskQueue;
	boost::mutex m_newTasks;

	boost::condition_variable m_wakeUp;
	boost::mutex m_wakeUpMutex;
	std::atomic<bool> m_stopped;

	void threadProc();
	bool popNextTask(boost::function<void()> & f);

public:
	ThreadPool(int number_of_threads);
	void addTask(boost::function<void()> f);
	void join();
	~ThreadPool();
};

