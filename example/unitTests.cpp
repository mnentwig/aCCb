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

#include "../aCCb/getFilesInDirectory.h"
#include "../aCCb/fileToString.h"
#include "../aCCb/splitToLines.h"
#include "../aCCb/stringToNum.h"
#include <set>
#include <algorithm>
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

TEST_START
	TEST_CASE("aCCb::stoi") {
		//try {} catch (std::exception & e){}
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

TEST_END
