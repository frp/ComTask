#include "ThreadPool.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
using namespace std;
using boost::thread;
using boost::lock_guard;
using boost::mutex;
using boost::unique_lock;

ThreadPool::ThreadPool(int number_of_threads) : m_threads(number_of_threads), m_stopped(false)
{
	for (int i = 0; i < number_of_threads; i++)
		m_threads[i] = thread(bind(&ThreadPool::threadProc, this));
}


ThreadPool::~ThreadPool()
{
	m_stopped = true;
	m_wakeUp.notify_all();
	for (thread & t : m_threads)
		t.join();
}

void ThreadPool::join()
{
	m_stopped = true;
	m_wakeUp.notify_all();
	for (thread & t : m_threads)
		t.join();
	m_stopped = false;
	for (int i = 0; i < m_threads.size(); i++)
		m_threads[i] = thread(bind(&ThreadPool::threadProc, this));
}

void ThreadPool::threadProc()
{
	bool done = false;
	while (!done)
	{
		boost::function<void()> task;
		if (!popNextTask(task))
			done = true;
		else
			task();
	}
}

bool ThreadPool::popNextTask(boost::function<void()> & f)
{
	for (;;)
	{
		// Check for new tasks
		{
			lock_guard<mutex> lg(m_queueMutex);
			if (!m_taskQueue.empty())
			{
				f = m_taskQueue.front();
				m_taskQueue.pop();
				return true;
			}
		}

		// Check if thread pool is stopped
		if (m_stopped) return false;
		
		// Wait for wakeup
		{
			unique_lock<mutex> ul(m_wakeUpMutex);
			m_wakeUp.wait(ul);
		}
	}
}

void ThreadPool::addTask(boost::function<void()> f)
{
	m_queueMutex.lock();
	m_taskQueue.push(f);
	m_queueMutex.unlock();
	m_wakeUp.notify_one();
}