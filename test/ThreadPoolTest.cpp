#include <gtest/gtest.h>
#include "ThreadPool.h"
#include <sstream>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace std;
using boost::posix_time::ptime;
using boost::posix_time::to_simple_wstring;
using boost::thread;
using boost::mutex;
using boost::lock_guard;

TEST(ThreadPoolTest, DoesTheDemandedTasks) {
	ThreadPool pool(2);
	int a = 0, b = 0, c = 0;
	pool.addTask([&]()->void { a = 1; });
	pool.addTask([&]()->void { b = 2; });
	pool.addTask([&]()->void { c = 3; });
	pool.join();
	EXPECT_EQ(a, 1);
	EXPECT_EQ(b, 2);
	EXPECT_EQ(c, 3);
}

TEST(ThreadPoolTest, CompletesAllTasksWhenDestroyed) {
	ThreadPool * pool = new ThreadPool(2);
	int a = 0, b = 0, c = 0;
	mutex m;
	m.lock(); // make all threads wait for mutex to be unlocked
	pool->addTask([&]()->void { lock_guard<mutex> g(m); a = 1; });
	pool->addTask([&]()->void { lock_guard<mutex> g(m); b = 2; });
	pool->addTask([&]()->void { lock_guard<mutex> g(m); c = 3; });
	thread t([&]()->void { delete pool; }); //start pool destroying
	ASSERT_EQ(a, 0);
	ASSERT_EQ(b, 0);
	ASSERT_EQ(c, 0);
	m.unlock(); // let threads proceed
	ASSERT_TRUE(t.timed_join(boost::posix_time::milliseconds(300)));
	// check results
	EXPECT_EQ(a, 1);
	EXPECT_EQ(b, 2);
	EXPECT_EQ(c, 3);
}
