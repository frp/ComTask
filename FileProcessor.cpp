#include "FileProcessor.h"
#include <fstream>
#include <string>
#include <boost/format.hpp>
using namespace std;
using boost::posix_time::ptime;

static ptime now()
{
	return boost::posix_time::second_clock::local_time();
}

static wstring humanReadableSize(int64_t size)
{
	static const int64_t kib = 1024;
	static const int64_t mib = kib * kib;
	static const int64_t gib = mib * kib;
	static const int64_t tib = gib * kib;
	using boost::wformat;
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

FileProcessor::FileProcessor(std::wofstream & log) : m_log(log)
{
}

void FileProcessor::processFileList(std::list<FileItem> & files)
{
	using namespace std;
	files.sort();
	m_log << "[ " << now() << "] Selection processing started"  << endl;
	for (auto & file : files)
	{
		processFile(file);
	}
	m_log << "[ " << now() << "] Selection processing finished" << endl;
}

void FileProcessor::processFile(const FileItem & file)
{
	uint32_t checkSum = calcCheckSum(file);
	m_log << "[ " << now() << " ] File processed" << endl
		<< "Name: " << file.name << endl
		<< "Size: " << humanReadableSize(file.size) << endl
		<< "Creation data: " << file.created_at << endl
		<< "Checksum: " << checkSum << " (0x" << hex << checkSum << ")" << dec << endl;
}

uint32_t FileProcessor::calcCheckSum(const FileItem & fi)
{
	uint32_t sum = 0;
	ifstream in(fi.full_name, ios::in | ios::binary);
	char c;
	while (in.get(c)) sum += static_cast<unsigned char>(c);
	return sum;
}