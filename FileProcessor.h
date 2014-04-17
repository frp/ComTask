#pragma once
#include <fstream>
#include <list>
#include <cstdint>

#include "FileItem.h"
#include "OrderedLogger.h"

class FileProcessor
{
	OrderedLogger & m_logger;
public:
	FileProcessor(OrderedLogger & logger);
	void processFileList(std::list<FileItem> & files);
	void processFile(const FileItem & file);
private:
	FileProcessor& operator=(const FileProcessor&);
	std::uint32_t calcCheckSum(const FileItem & fi);
};

