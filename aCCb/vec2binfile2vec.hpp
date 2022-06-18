#pragma once
#include <fstream> // ifstream
#include <vector>
#include <string>
#include <stdexcept>
namespace aCCb {
using std::string;
using std::vector;
using std::fstream;
using std::ifstream;
using std::runtime_error;
template<class T> std::vector<T> binfile2vec2(std::istream &is) {
	is.seekg(0, std::ios_base::end);
	std::size_t size_bytes = is.tellg();
	size_t nElements = size_bytes / sizeof(T);
	if (nElements * sizeof(T) != size_bytes)
		throw runtime_error("file contains partial element");
	is.seekg(0, std::ios_base::beg);
	vector<T> retVal(nElements);
	is.read((char*) &retVal[0], size_bytes);
	if (!is)
		throw runtime_error("read failed");
	return retVal;
}

template<class T> std::vector<T> binfile2vec(const string fname) {
	std::ifstream is(fname, std::ifstream::binary);
	if (!is)
		throw runtime_error("failed to open file");
	return binfile2vec2<T>(is);
}

template<class T> void binfile2vec(std::ostream &os, const vector<T> &vec) {
	os.write((char*) &vec[0], vec.size() * sizeof(T));
}

template<class T> void vec2binfile(const string &fname, const vector<T> &vec) {
	fstream os(fname.c_str(), std::ios::out | std::ifstream::binary);
	if (!os)
		throw runtime_error("failed to open file");
	binfile2vec<T>(os, vec);
}
} // NS
