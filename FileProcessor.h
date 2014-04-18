#pragma once
#include <fstream>
#include <list>
#include <cstdint>

#include "FileItem.h"
#include "OrderedLogger.h"

#include <boost/thread/mutex.hpp>

class FileProcessor
{
	OrderedLogger & m_logger;
public:
	FileProcessor(OrderedLogger & logger);
	void processFileList(std::list<FileItem> & files);
	void processFile(const FileItem & file, boost::mutex & waitMutex, boost::mutex & unlockMutex);
private:
	FileProcessor& operator=(const FileProcessor&);
	std::uint32_t calcCheckSum(const FileItem & fi);
};

