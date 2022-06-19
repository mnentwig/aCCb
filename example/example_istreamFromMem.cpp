#include <vector>
#include <numeric> // iota
#include <string>
#include <cassert>
#include "../aCCb/binIo.hpp"
#include "../aCCb/imemstream.hpp"
namespace binIo = aCCb::binaryIo;
#include <iostream>
using std::string;
using std::vector;
int main() {
	const string myFname = "tmp.bin";
	size_t myVecSize = 100;

	// === generate sample vector ===
	vector<uint64_t> vecA(myVecSize);
	std::iota(vecA.begin(), vecA.end(), 0);

	// === write to file ===
	binIo::vec2file(myFname, vecA);

	binIo::imemstream is((char*) &vecA[0], vecA.size() * sizeof(vecA[0]));
	vector<uint64_t> vecB = binIo::stream2vec<uint64_t>(is);
	assert(vecA == vecB);
	std::cout << "done\n" << std::flush;
}
