#pragma once
#include <fstream>
#include <string>
#include <boost/thread/mutex.hpp>
class OrderedLogger
{
	std::wostream & m_sink;
	bool m_timestamps;
	boost::mutex m_logmutex;
public:
	OrderedLogger(std::wostream & sink, bool timestamps);
	void operator()(std::wstring message, boost::mutex & ordering_mutex);
	void operator()(std::wstring message);
	static std::wstring generateLogName();
};

