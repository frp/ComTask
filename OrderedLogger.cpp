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

OrderedLogger::OrderedLogger(std::wostream & sink, bool timestamps)
: m_sink(sink), m_timestamps(timestamps), m_nextNumber(0)
{
}

void OrderedLogger::operator()(std::wstring message, int number)
{
	{
		lock_guard<mutex> lg(m_logmutex);
		m_records[number] = message;
	}
	flushRecords();
}

void OrderedLogger::flushRecords()
{
	lock_guard<mutex> lg(m_logmutex);
	while (!m_records.empty() && m_records.begin()->first == m_nextNumber)
	{
		printRecord(m_records.begin()->second);
		m_records.erase(m_records.begin());
		m_nextNumber++;
	}
}

void OrderedLogger::operator()(std::wstring message)
{
	boost::lock_guard<boost::mutex> guard_logging(m_logmutex);
	printRecord(message);
}

void OrderedLogger::printRecord(std::wstring message)
{
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