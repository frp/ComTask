#include "OrderedLogger.h"
#include <boost/thread/lock_guard.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <cstdlib>
using namespace std;
using boost::posix_time::ptime;
using boost::mutex;
using boost::lock_guard;

static ptime now()
{
	return boost::posix_time::second_clock::local_time();
}

OrderedLogger::OrderedLogger(std::wostream & sink, bool timestamps) : m_sink(sink), m_timestamps(timestamps)
{
}

void OrderedLogger::operator()(std::wstring message, boost::mutex & ordering_mutex)
{
	boost::lock_guard<boost::mutex> guard_order(ordering_mutex);
	operator()(message);
}

void OrderedLogger::operator()(std::wstring message)
{
	boost::lock_guard<boost::mutex> guard_logging(m_logmutex);
	if (m_timestamps)
		m_sink << now() << " ";
	m_sink << message << endl;
}

static const wstring chars_to_gen = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

wstring OrderedLogger::generateLogName()
{
	wstring result = L"log-";
	for (int i = 0; i < 7; i++)
		result += chars_to_gen[rand() % chars_to_gen.size()];
	return result + L".txt";
}