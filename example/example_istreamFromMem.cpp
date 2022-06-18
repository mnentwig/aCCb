#include <vector>
#include <numeric> // iota
#include <string>
#include <cassert>
#include "../aCCb/binIo.hpp"
namespace binIo = aCCb::binaryIo;
#include <iostream>
using std::vector;
using std::string;

#include <streambuf>
#include <istream>
struct membuf: std::streambuf {
	// prevent copying (also gets rid of -Weffc++ warnings)
	membuf(const membuf&) = delete;
	membuf& operator=(const membuf&) = delete;

	membuf(char *base, size_t size) :
			begin(base), end(base + size) {
		this->setg(this->begin, this->begin, this->end);
	}
	char *begin;
	char *end;
	virtual std::ios::pos_type seekoff(std::ios::off_type off, std::ios_base::seekdir dir, std::ios_base::openmode /*which = std::ios_base::in*/) override
	{
		if (dir == std::ios_base::cur)
			gbump(off);
		else if (dir == std::ios_base::end)
			setg(begin, end + off, end);
		else if (dir == std::ios_base::beg)
			setg(begin, begin + off, end);

		return gptr() - eback();
	}

	virtual std::ios::pos_type seekpos(std::streampos pos, std::ios_base::openmode mode) override
	{
		return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
	}
};
struct imemstream: virtual membuf, std::istream {
	imemstream(char const *base, size_t size) :
			membuf(const_cast<char*>(base), size), std::istream(static_cast<std::streambuf*>(this)) {
	}
};
int main() {
	const string myFname = "tmp.bin";
	size_t myVecSize = 100;

	// === generate sample vector ===
	vector<uint64_t> vecA(myVecSize);
	std::iota(vecA.begin(), vecA.end(), 0);

	// === write to file ===
	binIo::vec2file(myFname, vecA);

	imemstream is((char*) &vecA[0], vecA.size() * sizeof(vecA[0]));
	vector<uint64_t> vecB = binIo::stream2vec<uint64_t>(is);
	assert(vecA == vecB);
	std::cout << "done\n" << std::flush;
}
