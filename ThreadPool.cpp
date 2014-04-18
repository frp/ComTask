#include "ThreadPool.h"
#include <windows.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>
using namespace std;
using boost::thread;
using boost::lock_guard;
using boost::mutex;
using boost::unique_lock;

int getCountOfHardwareThreads()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

ThreadPool::ThreadPool() : ThreadPool(getCountOfHardwareThreads())
{
}

ThreadPool::ThreadPool(int number_of_threads) : m_numberOfThreads(number_of_threads)
{
	m_threads = new boost::thread_group;
	for (int i = 0; i < number_of_threads; i++)
		m_threads->create_thread(bind(&ThreadPool::threadProc, this));
}


ThreadPool::~ThreadPool()
{
	addTask(boost::function<void()>());
	m_threads->join_all();
	delete m_threads;
}

void ThreadPool::join()
{
	addTask(boost::function<void()>());
	m_threads->join_all();
	while (!m_taskQueue.empty())
		m_taskQueue.pop();
	delete m_threads;
	m_threads = new boost::thread_group;
	
	for (size_t i = 0; i < m_numberOfThreads; i++)
		m_threads->create_thread(bind(&ThreadPool::threadProc, this));
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