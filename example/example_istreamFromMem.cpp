#include <vector>
#include <numeric> // iota
#include <string>
#include "aCCb/vec2binfile2vec.hpp"
using std::vector;
using std::string;

#include <streambuf>
#include <istream>
struct membuf: std::streambuf {
	membuf(char const *base, size_t size) {
		char *p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
};
struct imemstream: virtual membuf, std::istream {
	imemstream(char const *base, size_t size) :
			membuf(base, size), std::istream(static_cast<std::streambuf*>(this)) {
	}
};
int main() {
	const string myFname = "tmp.bin";
	size_t myVecSize = 100;

	// === generate sample vector ===
	vector<uint64_t> vecA(myVecSize);
	std::iota(vecA.begin(), vecA.end(), 0);

	// === write to file ===
	aCCb::vec2binfile(myFname, vecA);

	imemstream((char*)&vecA[0], vecA.size());

}
