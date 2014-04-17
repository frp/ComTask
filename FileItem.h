#pragma once
#include <string>
#include <cstdint>
#include <boost/date_time/posix_time/posix_time.hpp>

struct FileItem
{
	std::wstring name;
	std::wstring full_name;
	std::int64_t size;
	boost::posix_time::ptime created_at;

	bool operator<(const FileItem & fi2) const
	{
		return name < fi2.name || (name == fi2.name && (size < fi2.size ||
			(size == fi2.size && created_at < fi2.created_at)));
	}
};

