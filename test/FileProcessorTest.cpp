#include <gtest/gtest.h>
#include "FileProcessor.h"
#include <sstream>
#include <cstdio>
#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace std;
using boost::posix_time::ptime;

static const wstring begin_str = L"Selection processing started";
static const wstring end_str = L"Selection processing finished";
static ptime now()
{
	return boost::posix_time::second_clock::local_time();
}

void createDummyFile(const char * name, initializer_list<uint8_t> data) {
	ofstream out(name);
	for (auto & c : data)
		out.put(c);
}

TEST(FileProcessorTest, ShouldLogProcessBeginningAndEnding) {
	wostringstream sink;
	FileProcessor processor(sink, false);
	list<FileItem> empty_list;
	processor.processFileList(empty_list);
	EXPECT_EQ(begin_str + L"\n" + end_str + L"\n", sink.str());
}

TEST(FileProcessorTest, ShouldLogInformationAboutFilesInAlphabetOrder) {
	wostringstream sink;
	FileProcessor processor(sink, false);

	createDummyFile("1", {1, 2, 3, 4});
	createDummyFile("2", {3, 4, 5, 6});

	list<FileItem> file_list;
	ptime t = now();
	file_list.push_back({ L"1", L"1", 4, t });
	file_list.push_back({ L"2", L"2", 4, t });
	processor.processFileList(file_list);
	EXPECT_EQ(begin_str + L"\n" + L"File processed\n\tName: 1\n\tSize: 4\n\tCreation data: "
		+to_simple_wstring(t)+L"\n\tChecksum: 10\n"
		+ L"File processed\n\tName: 2\n\tSize: 4\n\tCreation data: "
		+ to_simple_wstring(t) + L"\n\tChecksum: 18\n"
		+end_str + L"\n", sink.str());
}