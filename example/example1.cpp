#include "../aCCb/profiler.hpp"
aCCb::profiler gprof;
#include "../aCCb/stdIncludes.h"
#include "bnDataset.h"
#include "../aCCb/logicalIndexing.hpp"
#include <filesystem>

bool myStrCmp(const string &s1, const string &s2) {
	return s1.compare(s2) > 0;
}

void example_setCustomComparer(bnDataset &d) {

	// default sort
	std::set<string> namesForw(d.name.begin(), d.name.end());

	// custom sort in descending order
	std::set<string, decltype(&myStrCmp)> namesRev(d.name.begin(), d.name.end(), &myStrCmp);

	// alternative version, with lambda expression
	auto myStrCmpLambda = [](const string &s1, const string &s2) {
		return s1.compare(s2) > 0;
	};
	std::set<string, decltype(myStrCmpLambda)> namesRev2(myStrCmpLambda);
	namesRev2.insert(d.name.begin(), d.name.end());

	auto itForw = namesForw.cbegin();
	auto itForwEnd = namesForw.cend();
	auto itRev = namesRev.crbegin();
	auto itRev2 = namesRev2.crbegin();
#ifndef NDEBUG
	auto itRevEnd = namesRev.crend();
	auto itRev2End = namesRev2.crend();
#endif
	while (itForw != itForwEnd) {
		assert(itRev != itRevEnd);
		assert(itRev2 != itRev2End);
		string nForw = *itForw;
		string nRev = *itRev;
		string nRev2 = *itRev2;
		assert(nForw == nRev);
		assert(nForw == nRev2);
		if (nForw != nRev)
			cerr << "list reversal failed\n" << std::flush;
		if (nForw != nRev2)
			cerr << "list reversal failed\n" << std::flush;
	}
}
#include <numeric> // iota

#include <ios>
#include <istream>
#include <iostream>
#if 0
struct membuf : std::streambuf
  {
    membuf(char *begin, char *end) : begin(begin), end(end)
    {
      this->setg(begin, begin, end);
    }

    virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) override
    {
      if(dir == std::ios_base::cur)
        gbump(off);
      else if(dir == std::ios_base::end)
        setg(begin, end+off, end);
      else if(dir == std::ios_base::beg)
        setg(begin, begin+off, end);

      return gptr() - eback();
    }

    virtual pos_type seekpos(std::streampos pos, std::ios_base::openmode mode) override
    {
      return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
    }

    char *begin, *end;
  };

  std::istream *myStream;

  /* from memory.. */
 char buf[100];
  membuf sbuf(&buf[0], &buf[100]);
  std::istream in(&sbuf);
   myStream = &in;

  /* or from file */
  std::ifstream file("myFilename", std::ios::binary);
  myStream = &file;
#endif

#include <streambuf>
#include <istream>

struct membuf: std::streambuf {
	membuf(char const *base, size_t size) {
		char *p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
};
struct myMembuf_t: std::streambuf {
	myMembuf_t(char const *base, size_t size) {
		char *p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
};
struct imemstream: virtual membuf, std::istream {
	imemstream(char const *base, size_t size) :
			membuf(base, size), std::istream(static_cast<std::streambuf*>(this)) {
	}
};

#if 0
void hello(){
char buf[100];
 myMembuf_t myBuf(&buf[0], 100);
 myBuf.setg(&buf[0], &buf[0], &buf[100]);
 std::istream myMemstream(&myMembuf);
 std::ifstream file("myFilename", std::ios::binary);

 std::istream s = file;
 std::istream s2 = myMemstream;
}
#endif
namespace li = aCCb::logicalIndexing;
int mainB() {

	vector<uint64_t> testvecA(10);
	std::iota(testvecA.begin(), testvecA.end(), 0);
	vector<int64_t> testvecB(10);
	std::iota(testvecB.begin(), testvecB.end(), 1);

	bool (*expr)(const uint64_t&) = [](const uint64_t &v) {
		return (bool)(v % 2 == 0);
	};

	vector<bool> li = li::generateIndex(testvecA, expr);
	vector<uint64_t> testvecA2 = li::applyIndex(testvecA, li);
	vector<int64_t> testvecB2 = li::applyIndex(testvecB, li);
	//for (auto e : testvec2)
	//	cout << e << "\n" << std::flush;

	li::vecMap m;
	m.set("tvA", testvecA2);
	m.set("tvB", testvecB2);
	//m.getData_uint64("hello");
	//vector<uint64_t> retval = m.getData_uint8<uint64_t>("hello");
	vector<uint64_t> &vA = m.get("tvA");
	for (auto e : vA)
		cout << e << "\n" << std::flush;
	vector<int64_t> &vB = m.get("tvB");
	for (auto e : vB)
		cout << e << "\n" << std::flush;
	return 0;
}

int main() {
	cout << "ex1\n";
	cout << "current folder: " << std::filesystem::current_path() << endl;

	try {
		return mainB();
	} catch (std::exception &e) {
		std::cerr << "exception:" << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "unspecific exception" << std::endl;
	}
}
