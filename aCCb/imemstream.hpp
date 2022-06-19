#pragma once
//#include <streambuf>
#include <istream>

namespace aCCb::binaryIo {
namespace details {
struct membuf: std::streambuf {
	// prevent copying (also gets rid of -Weffc++ warnings)
	membuf(const membuf&) = delete;
	membuf& operator=(const membuf&) = delete;

	membuf(const char *base, size_t size) :
			begin(base), end(base + size) {
		this->setg(const_cast<char*>(this->begin), const_cast<char*>(this->begin), const_cast<char*>(this->end));
	}
	const char *begin;
	const char *end;
	virtual std::ios::pos_type seekoff(std::ios::off_type off, std::ios_base::seekdir dir, std::ios_base::openmode /*which = std::ios_base::in*/) override
	{
		if (dir == std::ios_base::cur)
			gbump(off);
		else if (dir == std::ios_base::end)
			setg(const_cast<char*>(begin), const_cast<char*>(end) + off, const_cast<char*>(end));
		else if (dir == std::ios_base::beg)
			setg(const_cast<char*>(begin), const_cast<char*>(begin) + off, const_cast<char*>(end));

		return gptr() - eback();
	}

	virtual std::ios::pos_type seekpos(std::streampos pos, std::ios_base::openmode mode) override
	{
		return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
	}
};
} // NS: details

/** istream fron binary data */
struct imemstream: virtual details::membuf, std::istream {
	imemstream(char const *base, size_t size) :
			membuf(const_cast<char*>(base), size), std::istream(static_cast<std::streambuf*>(this)) {
	}
};
} // NS: aCCb::binaryIo
