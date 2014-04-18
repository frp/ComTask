#include "ThreadPool.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <iostream>
using namespace std;
using boost::thread;
using boost::lock_guard;
using boost::mutex;
using boost::unique_lock;

ThreadPool::ThreadPool(int number_of_threads) : m_threads(number_of_threads)
{
	for (int i = 0; i < number_of_threads; i++)
		m_threads[i] = thread(bind(&ThreadPool::threadProc, this));
}


ThreadPool::~ThreadPool()
{
	/*m_stopped = true;
	m_wakeUp.notify_all();*/
	addTask(boost::function<void()>());
	for (thread & t : m_threads)
		t.join();
}

void ThreadPool::join()
{
	addTask(boost::function<void()>());
	for (thread & t : m_threads)
		t.join();
	while (!m_taskQueue.empty())
		m_taskQueue.pop();
	for (int i = 0; i < m_threads.size(); i++)
		m_threads[i] = thread(bind(&ThreadPool::threadProc, this));
}

void ThreadPool::threadProc()
{
	for (;;)
	{
		boost::function<void()> task;
		{
			unique_lock<mutex> q_lock(m_queueMutex);
			while (m_taskQueue.empty())
				m_wakeUp.wait(q_lock);
			task = m_taskQueue.front();
			m_taskQueue.pop();
		}
		if (task.empty())
		{
			addTask(task);
			break;
		}
		else
			task();
	}
}

void ThreadPool::addTask(boost::function<void()> f)
{
	m_queueMutex.lock();
	m_taskQueue.push(f);
	m_queueMutex.unlock();
	m_wakeUp.notify_one();
}