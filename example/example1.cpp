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
	vector<uint64_t>& vA = m.get("tvA");
	for (auto e : vA)
		cout << e << "\n" << std::flush;
	vector<int64_t>& vB = m.get("tvB");
	for (auto e : vB)
		cout << e << "\n" << std::flush;
	return 0;
	auto vC = m.get("tvB");
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
