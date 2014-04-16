#include "FileProcessor.h"
#include <fstream>

FileProcessor::FileProcessor(std::wofstream & log) : m_log(log)
{
}

void FileProcessor::processFileList(std::list<FileItem> & files)
{
	using namespace std;
	files.sort();
	m_log << "Items:" << endl;
	for (auto & file : files)
	{
		m_log << file.name << "\n";
	}
}
