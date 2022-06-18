#if 0
#	define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#	include "../unitTestFrameworks/doctest.h"
#elif 0
#	define CATCH_CONFIG_MAIN
#	define CATCH_CONFIG_FAST_COMPILE
#	include "../unitTestFrameworks/catch.hpp"
#endif

// always include own framework for added macros
// (does nothing if doctest or catch2 are detected)
#include "../aCCb/unitTestFramework.h"

#include "../aCCb/getFilesInDirectory.hpp"
#include "../aCCb/fileToString.hpp"
#include "../aCCb/splitToLines.hpp"
#include "../aCCb/stringToNum.hpp"

#include "../aCCb/vec2binfile2vec.hpp"
#include "../aCCb/logicalIndexing.hpp"

#if true
// the tested libraries may have profiling enabled somewhere.
// we need to provide a profiler object (otherwise linker error)
// if profiling is not used anywhere, this can be omitted
#include "../aCCb/profiler.hpp"
aCCb::profiler gprof;
#endif

#include <set>
#include <algorithm>
#include <numeric> // iota
using std::string;
using std::regex;
using std::unordered_set;
using std::endl;
using std::vector;

// example function defined in a different object file (makefile test)
int objCppExampleForMakefile();

int strequal(const string &s1, const string &s2) {
	return s1.compare(s2) > 0;
}

int testGetFilesInDirectory(string pat, int firstYear, int lastYear) {
	const regex r(pat);
	unordered_set<string> files = aCCb::getFilesInDirectory("sampledataHistoricalBabynames",
	/* regex */r,
	/* include files */true,
	/* include directories */false,
	/* keep path in results */false);

	// === construct reference result ===
	unordered_set<string> ref;
	for (int year = firstYear; year <= lastYear; ++year) {
		string n("yob");
		n += std::to_string(year);
		n += ".txt";
		ref.insert(n);
	}

	// === check results for same content ===
	// input can be in any order
	// std::set_difference needs ordered input!
	std::unordered_set<string> diff;
	std::set<string> filesSorted(files.begin(), files.end());
	std::set<string> refSorted(ref.begin(), ref.end());
	std::set_difference( 									//
			filesSorted.begin(), filesSorted.end(), 		// sorted input 1
			refSorted.begin(), refSorted.end(), 			// sorted input 2
			std::inserter(diff, diff.begin()));				// output

#if 0
	auto cond = [](const string &i) {
		bool res = i.compare("yob1880.txt") == 0;
		return res;
	};

	cout << (std::find_if(filesSorted.begin(), filesSorted.end(), cond) != filesSorted.end()) << endl;
	cout << (std::find_if(refSorted.begin(), refSorted.end(), cond) != refSorted.end()) << endl;
	cout << (std::find_if(diff.begin(), diff.end(), cond) != diff.end()) << endl;
	for (auto x : filesSorted)
		std::cout << "files\t'" << x << "'" << std::endl;
	for (auto x : diff)
		std::cout << "diff\t'" << x << "'" << std::endl;
	for (auto x : refSorted)
		std::cout << "ref\t'" << x << "'" << std::endl;
	std::cout << "-------------" << std::endl;
#endif
	return diff.size();
}

bool testSplitToLines() {
	string contents = aCCb::fileToString("sampledataHistoricalBabynames/yob1880.txt");
	vector<string> linesA = aCCb::splitToLines(contents);
	vector<string> linesB = aCCb::splitToLines_STL(contents);

	if (linesA != linesB)
		return false;

	aCCb::removeLastDummyLine(linesA);
	aCCb::removeLastDummyLine(linesB);
	return (linesA == linesB);
}

bool testVec2binfile2vec() {
	vector<uint8_t> testvec8(256);
	vector<uint16_t> testvec16(1000);
	vector<uint32_t> testvec32(1000);
	vector<uint64_t> testvec64(1000);

	std::iota(testvec8.begin(), testvec8.end(), 0);
	std::iota(testvec16.begin(), testvec16.end(), 0);
	std::iota(testvec32.begin(), testvec32.end(), 0);
	std::iota(testvec64.begin(), testvec64.end(), 0);
	const string fname("testfile.bin");

	bool pass = true;

	aCCb::vec2binfile(fname, testvec8);
	pass &= (aCCb::binfile2vec<uint8_t>(fname) == testvec8);

	aCCb::vec2binfile(fname, testvec16);
	pass &= (aCCb::binfile2vec<uint16_t>(fname) == testvec16);

	aCCb::vec2binfile(fname, testvec32);
	pass &= (aCCb::binfile2vec<uint32_t>(fname) == testvec32);

	aCCb::vec2binfile(fname, testvec64);
	pass &= (aCCb::binfile2vec<uint64_t>(fname) == testvec64);

	return pass;
}

bool testLogicalIndexing() {
	// === test vector ===
	vector<uint32_t> v(1000);
	std::iota(v.begin(), v.end(), 0);
	const string fname("testLi.bin");
	aCCb::vec2binfile(fname, v);

	// === generateIndex 1-arg: Even values ===
	auto expr1 = [](const uint32_t &val) {
		return val % 2 == 0;
	};
	vector<bool> indexOp1 = li::generateIndex<uint32_t>(v, expr1);
	bool pass = true;
	pass &= li::popcount(indexOp1) == v.size() / 2;

	// === generateIndex 2-arg: Values 10, 11, ... ===
	auto expr2 = [](const uint32_t&/*val*/, size_t pos) {
		return pos >= 10;
	};
	vector<bool> indexOp2 = li::generateIndex<uint32_t>(v, expr2);
	pass &= li::popcount(indexOp2) == v.size() - 10;

	// === logical not ===
	auto indexOp1Neg = li::logicalNot(indexOp1);
	pass &= li::popcount(indexOp1Neg) == v.size() / 2;

	// === logical or ===
	pass &= li::popcount(li::logicalOr(indexOp1, indexOp1Neg)) == v.size();
	pass &= li::popcount(li::logicalOr(indexOp1, indexOp2)) == v.size() - 5;

	// === logical and ===
	pass &= li::popcount(li::logicalAnd(indexOp1, indexOp1Neg)) == 0;
	pass &= li::popcount(li::logicalAnd(indexOp1, indexOp2)) == v.size() / 2 - 5;

	// === binary file read with indexOp ===
	//pass &= aCCb::binfile2vec<uint32_t>(fname, indexOp1Neg) == li::applyIndex(v, indexOp1Neg);


	return pass;
}

TEST_START
	TEST_CASE("aCCb::stoi") {
		REQUIRE(aCCb::stoi("123") == 123);
		REQUIRE(aCCb::stoi("-10") == -10);
		CHECK_THROWS_WITH(aCCb::stoi("0xFFFF"), "conversion string to number failed");
	}

	TEST_CASE("aCCb::getFilesInDirectory") {
		REQUIRE(testGetFilesInDirectory(R"(^.*yob\d\d\d\d.txt$)", 1880, 2021) == 0);
		REQUIRE(testGetFilesInDirectory(R"(^.*yob\d\d\d\d.txt$)", 1880, 2021 - 1) == 1);
		REQUIRE(testGetFilesInDirectory(R"(^.*yob18\d\d.txt$)", 1880, 1899) == 0);
		REQUIRE(testGetFilesInDirectory(R"(^.*yob19\d\d.txt$)", 1900, 1999) == 0);
	}

	TEST_CASE("link with object") {
		REQUIRE(objCppExampleForMakefile() == 42);
	}

	TEST_CASE("text file") {
		REQUIRE(testSplitToLines() == true);
	}

	TEST_CASE("vec2binfile2vec") {
		REQUIRE(testVec2binfile2vec());
	}

	TEST_CASE("logicalIndexing") {
		REQUIRE(testLogicalIndexing());
	}

	TEST_END
