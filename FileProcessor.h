#pragma once
#include <fstream>
#include <list>
#include <cstdint>
#include <ostream>

#include "FileItem.h"
#include "OrderedLogger.h"

#include <boost/thread/mutex.hpp>

class FileProcessor
{
	OrderedLogger m_logger;
	std::size_t m_nextIndex;
public:
	FileProcessor(std::wostream & sink, bool timestamps);
	void processFileList(std::list<FileItem> & files);
private:
	void processFile(const FileItem & file, int number);
	std::uint32_t calcCheckSum(const FileItem & fi);
};

