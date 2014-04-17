#include "OrderedLogger.h"
#include <boost/thread/lock_guard.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
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