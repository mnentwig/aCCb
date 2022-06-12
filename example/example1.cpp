#include "../aCCb/profiler.hpp"
aCCb::profiler gprof;
#include "../aCCb/stdIncludes.h"
#include "bnDataset.h"
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
		if(nForw != nRev) cerr << "list reversal failed\n" << std::flush;
		if(nForw != nRev2) cerr << "list reversal failed\n" << std::flush;
	}
}

int mainB() {
	bnDataset d(string("sampledataHistoricalBabynames"));
	cout << gprof.report();
	return 0;

	example_setCustomComparer(d);
	cout << "Example done\n";
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
