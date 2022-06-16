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
template<class T> std::vector<T> binfile2vec(const string fname) {
	ifstream is(fname, std::ifstream::binary);
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
template<class T> void vec2binfile(const string &fname, const vector<T> &vec) {
	fstream os(fname.c_str(), std::ios::out | std::ifstream::binary);
	os.write((char*) &vec[0], vec.size() * sizeof(T));
	os.close();
	if (!os)
		throw runtime_error("file contains partial element");
}
}
