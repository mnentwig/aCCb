#pragma once
#include <fstream> // ifstream
#include <vector>
#include <string>
#include <stdexcept>
#include "../aCCb/logicalIndexing.hpp"
namespace li = aCCb::logicalIndexing;

namespace aCCb {
using std::string;
using std::vector;
using std::fstream;
using std::ifstream;
using std::runtime_error;

inline size_t binfileGetNElem(std::istream &is, size_t elemSize) {
	is.seekg(0, std::ios_base::end);
	std::size_t nBytes = is.tellg();
	size_t nElem = nBytes / elemSize;
	if (nElem * elemSize != nBytes)
		throw runtime_error("file contains partial element");
	return nElem;
}

template<class T> std::vector<T> binfile2vec2(std::istream &is) {
	size_t nElem = binfileGetNElem(is, sizeof(T));
	is.seekg(0, std::ios_base::beg);
	vector<T> retVal(nElem);
	is.read((char*) &retVal[0], nElem * sizeof(T));
	if (!is)
		throw runtime_error("read failed");
	return retVal;
}
template<class T> std::vector<T> binfile2vec2(std::istream &is, std::vector<bool> indexOp) {
	size_t nElem = binfileGetNElem(is, sizeof(T));
	if (nElem != indexOp.size())
		throw runtime_error("size mismatch is vs indexOp");

	// === reserve memory ===
	size_t popcnt = li::popcount(indexOp);
	vector<T> retVal(popcnt);

	// === main loop ===
	auto itIndex = indexOp.cbegin();
	const auto itIndexEnd = indexOp.cend();
	size_t streamBytePos = 0;
	size_t ixRead = 0;
	while (itIndex != itIndexEnd) {
		// === skip consecutive false ===
		while ((itIndex != itIndexEnd) && !*itIndex) {
			streamBytePos += sizeof(T);
			++itIndex;
		}

		// === count consecutive true ===
		size_t nElemToRead = 0;
		while ((itIndex != itIndexEnd) && *itIndex) {
			++nElemToRead;
			++itIndex;
		}

		// === read contiguous elements ===
		if (nElemToRead) {
			size_t nBytesToRead = nElemToRead * sizeof(T);
			is.seekg(streamBytePos, std::ios_base::beg);
			if (!is)
				throw runtime_error("seekg() failed");
			is.read((char*) &retVal[ixRead], nBytesToRead);
			if (!is)
				throw runtime_error("read() failed");
			ixRead += nElemToRead;
			streamBytePos += nBytesToRead;
		}

		// === done? (optimization for trailing false) ===
		if (popcnt == ixRead)
			break;
	}
	return retVal;
}

template<class T> std::vector<T> binfile2vec(const string fname) {
	std::ifstream is(fname, std::ifstream::binary);
	if (!is)
		throw runtime_error("failed to open file");
	return binfile2vec2<T>(is);
}

template<class T> std::vector<T> binfile2vec(const string fname, const vector<bool>& indexOp) {
	std::ifstream is(fname, std::ifstream::binary);
	if (!is)
		throw runtime_error("failed to open file");
	return binfile2vec2<T>(is, indexOp);
}

template<class T> void vec2binfile(std::ostream &os, const vector<T> &vec) {
	os.write((char*) &vec[0], vec.size() * sizeof(T));
}

template<class T> void vec2binfile(const string &fname, const vector<T> &vec) {
	fstream os(fname.c_str(), std::ios::out | std::ifstream::binary);
	if (!os)
		throw runtime_error("failed to open file");
	vec2binfile<T>(os, vec);
}
} // NS
