#pragma once
#include <fstream>
#include <string>
#include <boost/thread/mutex.hpp>
#include <vector>
#include <map>
class OrderedLogger
{
	std::wostream & m_sink;
	bool m_timestamps;
	boost::mutex m_logmutex;
	std::map<size_t, std::wstring> m_records;
	int m_nextNumber;

	void flushRecords();
	void printRecord(std::wstring message);
public:
	OrderedLogger(std::wostream & sink, bool timestamps);
	void operator()(std::wstring message, size_t record_number);
	void operator()(std::wstring message);
	static std::wstring generateLogName();
};

