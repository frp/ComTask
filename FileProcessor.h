#pragma once
#include <fstream>
#include <list>
#include "FileItem.h"
#include <cstdint>

class FileProcessor
{
	std::wofstream & m_log;
public:
	FileProcessor(std::wofstream & log);
	void processFileList(std::list<FileItem> & files);
	void processFile(const FileItem & file);
private:
	FileProcessor& operator=(const FileProcessor&);
	std::uint32_t calcCheckSum(const FileItem & fi);
};

