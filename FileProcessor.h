#pragma once
#include <fstream>
#include <list>
#include "FileItem.h"

class FileProcessor
{
	std::wofstream & m_log;
public:
	FileProcessor(std::wofstream & log);
	void processFileList(std::list<FileItem> & files);
private:
	FileProcessor& operator=(const FileProcessor&);
};

