#include "FileProcessor.h"
#include <fstream>
#include <string>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
using namespace std;
using boost::posix_time::ptime;
using boost::mutex;
using boost::wformat;

static wstring humanReadableSize(int64_t size)
{
	static const int64_t kib = 1024;
	static const int64_t mib = kib * kib;
	static const int64_t gib = mib * kib;
	static const int64_t tib = gib * kib;
	if (size < kib)
		return str(wformat(L"%1%") % size);
	else if (size < mib)
		return str(wformat(L"%.2f KB") % (double(size) / kib));
	else if (size < gib)
		return str(wformat(L"%.2f MB") % (double(size) / mib));
	else if (size < tib)
		return str(wformat(L"%.2f GB") % (double(size) / gib));
	else
		return str(wformat(L"%.2f TB") % (double(size) / tib));
}

FileProcessor::FileProcessor(OrderedLogger & logger) : m_logger(logger){}

void FileProcessor::processFileList(std::list<FileItem> & files)
{
	files.sort();
	m_logger(L"Selection processing started");
	for (auto & file : files)
	{
		processFile(file);
	}
	m_logger(L"Selection processing finished");
}

void FileProcessor::processFile(const FileItem & file)
{
	uint32_t checkSum = calcCheckSum(file);
	m_logger(str(wformat(L"File processed\n\tName: %1%\n\tSize: %2%\n\tCreation data: %3%\n\tChecksum: %4%")
		% file.name % humanReadableSize(file.size) % file.created_at % checkSum)); //<< " (0x" << hex << checkSum << ")" << dec << endl;
}

uint32_t FileProcessor::calcCheckSum(const FileItem & fi)
{
	uint32_t sum = 0;
	ifstream in(fi.full_name, ios::in | ios::binary);
	char c;
	while (in.get(c)) sum += static_cast<unsigned char>(c);
	return sum;
}