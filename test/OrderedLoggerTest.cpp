#include <gtest/gtest.h>
#include "OrderedLogger.h"
#include <sstream>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace std;
using boost::posix_time::ptime;
using boost::posix_time::to_simple_wstring;
using boost::thread;
using boost::mutex;

static const wstring record1 = L"My cool log record";
static const wstring record2 = L"Yet another cool log record";
static const wstring expectation1 = record1 + L"\n";
static const wstring expectation2 = expectation1 + record2 + L"\n";

static ptime now()
{
	return boost::posix_time::second_clock::local_time();
}

TEST(OrderedLoggerTest, LogsUnsynchronized) {
	wostringstream sink;
	OrderedLogger logger(sink, false); // don't log timestamps
	EXPECT_EQ(L"", sink.str()); // On initialization, logger does not log anything at all
	logger(record1);
	EXPECT_EQ(expectation1, sink.str());
	logger(record2);
	EXPECT_EQ(expectation2, sink.str());
}

TEST(OrderedLoggerTest, LogsTimestamps) {
	wostringstream sink;
	OrderedLogger logger(sink, true);
	logger(record1);
	EXPECT_EQ(to_simple_wstring(now()) + L" " + expectation1, sink.str());
	logger(record2);
	EXPECT_EQ(to_simple_wstring(now()) + L" " + expectation1 + to_simple_wstring(now())
		+ L" " + record2 + L"\n", sink.str());
}

TEST(OrderedLoggerTest, LogsPreservingOrder) {
	using namespace boost;
	wostringstream sink;
	OrderedLogger logger(sink, false);
	mutex mutex1, mutex2;
	mutex1.lock(); // prevent record2 from logging immediately
	thread log_thread1([&]()->void{ logger(record2, mutex1); });
	thread log_thread2([&]()->void{ logger(record1, mutex2); });
	ASSERT_TRUE(log_thread2.timed_join(posix_time::milliseconds(100)));
	EXPECT_EQ(expectation1, sink.str());
	mutex1.unlock(); // let record2 be logged
	ASSERT_TRUE(log_thread1.timed_join(posix_time::milliseconds(100)));
	EXPECT_EQ(expectation2, sink.str());
}