#if 0
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../unitTestFrameworks/doctest.h"
#elif 0
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE
#include "../unitTestFrameworks/catch.hpp"
#endif

#include <algorithm>
#include <cmath>    // fabs
#include <numeric>  // iota
#include <set>
#include <sstream>

// always include own framework for added macros
// (does nothing if doctest or catch2 are detected)
#include "../aCCb/binIo.hpp"
#include "../aCCb/fileToString.hpp"
#include "../aCCb/getFilesInDirectory.hpp"
#include "../aCCb/imemstream.hpp"
#include "../aCCb/splitToLines.hpp"
#include "../aCCb/stringToNum.hpp"
#include "../aCCb/unitTestFramework.h"
namespace binIo = aCCb::binaryIo;

#include "../aCCb/logicalIndexing.hpp"
namespace li = aCCb::logicalIndexing;

#if true
// the tested libraries may have profiling enabled somewhere.
// we need to provide a profiler object (otherwise linker error)
// if profiling is not used anywhere, this can be omitted
#include "../aCCb/profiler.hpp"
aCCb::profiler gprof;
#endif

using std::endl, std::regex, std::string, std::unordered_set, std::vector;
// example function defined in a different object file (makefile test)
int objCppExampleForMakefile();

int strequal(const string &s1, const string &s2) {
    return s1.compare(s2) > 0;
}

int testGetFilesInDirectory(string pat, int firstYear, int lastYear) {
    const regex r(pat);
    unordered_set<string> files = aCCb::getFilesInDirectory("sampledataHistoricalBabynames",
                                                            /* regex */ r,
                                                            /* include files */ true,
                                                            /* include directories */ false,
                                                            /* keep path in results */ false);

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
    std::set_difference(                         //
        filesSorted.begin(), filesSorted.end(),  // sorted input 1
        refSorted.begin(), refSorted.end(),      // sorted input 2
        std::inserter(diff, diff.begin()));      // output

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

    binIo::vec2file(fname, testvec8);
    pass &= (binIo::file2vec<uint8_t>(fname) == testvec8);

    binIo::vec2file(fname, testvec16);
    pass &= (binIo::file2vec<uint16_t>(fname) == testvec16);

    binIo::vec2file(fname, testvec32);
    pass &= (binIo::file2vec<uint32_t>(fname) == testvec32);

    binIo::vec2file(fname, testvec64);
    pass &= (binIo::file2vec<uint64_t>(fname) == testvec64);

    return pass;
}

template <typename T>
bool test_imemstream() {
    // === generate sample vector ===
    vector<T> vecRef(100);
    std::iota(vecRef.begin(), vecRef.end(), 0);

    // === create imemstream ===
    size_t nBytes = vecRef.size() * sizeof(vecRef[0]);
    const char *dataPtr = (const char *)&vecRef[0];
    binIo::imemstream myInStream(dataPtr, nBytes);

    // === read back from imemstream to vec ===
    vector<T> vecB = binIo::stream2vec<T>(myInStream);

    return vecB == vecRef;
}

template <typename T>
bool testMaskedWriteRead(const vector<T> &v, const string fname, const vector<bool> &indexOp) {
    string fnameTmp = "tmp.bin";
    bool pass = true;
    vector<T> refResult = li::applyIndex(v, indexOp);
    binIo::vec2file<T>(fnameTmp, v, indexOp);            // masked write
    vector<T> vReadback = binIo::file2vec<T>(fnameTmp);  // ... and read back
    pass &= vReadback == li::applyIndex(v, indexOp);     // check

    vReadback = binIo::file2vec<T>(fname, indexOp);  // masked read
    pass &= vReadback == refResult;                  // check
    return pass;
}

template <typename T>
bool testLogicalIndexing() {
    // === test vector ===
    vector<T> v(1000);
    std::iota(v.begin(), v.end(), 0);
    const string fname("testLi.bin");
    binIo::vec2file(fname, v);

    // === generateIndex 1-arg: Even values ===
    auto expr1 = [](const T &val) {
        return val % 2 == 0;
    };
    vector<bool> indexOp1 = li::generateIndex<T>(v, expr1);
    bool pass = true;
    pass &= li::popcount(indexOp1) == v.size() / 2;

    // === generateIndex 2-arg: Values 10, 11, ... ===
    auto expr2 = [](const T & /*val*/, size_t pos) {
        return pos >= 10;
    };
    vector<bool> indexOp2 = li::generateIndex<T>(v, expr2);
    pass &= li::popcount(indexOp2) == v.size() - 10;

    // === logical not ===
    auto indexOp1Neg = li::logicalNot(indexOp1);
    pass &= li::popcount(indexOp1Neg) == v.size() / 2;

    // === logical or ===
    vector<bool> allTrue = li::logicalOr(indexOp1, indexOp1Neg);
    pass &= li::popcount(allTrue) == v.size();
    vector<bool> allButFive = li::logicalOr(indexOp1, indexOp2);
    pass &= li::popcount(allButFive) == v.size() - 5;

    // === logical and ===
    vector<bool> allFalse = li::logicalAnd(indexOp1, indexOp1Neg);
    pass &= li::popcount(allFalse) == 0;
    vector<bool> halfMinusFive = li::logicalAnd(indexOp1, indexOp2);
    pass &= li::popcount(halfMinusFive) == v.size() / 2 - 5;

    // === binary file read with indexOp ===
    pass &= binIo::file2vec<T>(fname, indexOp1Neg) == li::applyIndex(v, indexOp1Neg);
    pass &= binIo::file2vec<T>(fname, indexOp1) == li::applyIndex(v, indexOp1);
    pass &= binIo::file2vec<T>(fname, indexOp2) == li::applyIndex(v, indexOp2);

    // === masked write / read ===
    pass &= testMaskedWriteRead<T>(v, fname, allFalse);
    pass &= testMaskedWriteRead<T>(v, fname, allTrue);
    pass &= testMaskedWriteRead<T>(v, fname, allButFive);
    pass &= testMaskedWriteRead<T>(v, fname, halfMinusFive);
    return pass;
}

#if 0
TEST(IsNumber, True) {
	char const *tests[] {"42", "3.14", "-0", "+4", ".3",
		"+.5", "-.23", "7.", "1e2", "1.e2",
		"1.0e-2", "8.e+09", "2E34", "61e2", "-0e1",
		"+0E+10", "-.01E-5", "07", "+01E1", "12.34"};
	for (auto const &x : tests) {
		EXPECT_TRUE(is_number(x));
	}
}

TEST(IsNumber, False) {
	char const *tests[] {"4e", "xyz", ".3.14", "--0", "2-4",
		"..3", ".+5", "7 2", "1f", "1.0f",
		"1e-2.0", "8e+0e1", "2E.4", "a", "e15",
		"-0e10.3", ".e2", "+1.2E0e", "1.2+3", "e1"};
	for (auto const &x : tests) {
		EXPECT_FALSE(is_number(x));
	}
}
#endif

#undef NDEBUG  // Note: standard supports re-inclusion of <assert> with NDEBUG changed
#include <cassert>
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

TEST_CASE("binIo::vec to file to vec") {
    REQUIRE(testVec2binfile2vec());
}

TEST_CASE("binIo::imemstream") {
    REQUIRE(test_imemstream<uint8_t>());
    REQUIRE(test_imemstream<uint64_t>());
    REQUIRE(test_imemstream<float>());
}

TEST_CASE("logicalIndexing") {
    REQUIRE(testLogicalIndexing<int16_t>());
    REQUIRE(testLogicalIndexing<uint64_t>());
}

TEST_CASE("str2num") {
    uint8_t valu8;
    REQUIRE(!aCCb::str2num("-1", valu8));
    REQUIRE(aCCb::str2num("0", valu8) && (valu8 == 0));
    REQUIRE(aCCb::str2num("255", valu8) && (valu8 == 255));
    REQUIRE(!aCCb::str2num("256", valu8));
    int8_t val8;
    REQUIRE(!aCCb::str2num("-129", val8));
    REQUIRE(aCCb::str2num("-128", val8) && (val8 == -128));
    REQUIRE(aCCb::str2num("127", val8) && (val8 == 127));
    REQUIRE(!aCCb::str2num("128", val8));
    int16_t val16;
    REQUIRE(aCCb::str2num("-32768", val16) && (val16 == -32768));
    REQUIRE(!aCCb::str2num("-32769", val16));
    REQUIRE(aCCb::str2num("32767", val16) && (val16 == 32767));
    REQUIRE(!aCCb::str2num("32768", val16));
    REQUIRE(!aCCb::str2num("987654321098765432109876543210", val16));
    REQUIRE(aCCb::str2num("12345", val16) && (val16 == 12345));
    int32_t val32;
    uint32_t valu32;
    REQUIRE(aCCb::str2num("-12345", val32) && (val32 == -12345));
    REQUIRE(!aCCb::str2num("-12345", valu32));
    bool valBool;
    REQUIRE(aCCb::str2num("0", valBool) && (valBool == false));
    REQUIRE(aCCb::str2num("1", valBool) && (valBool == true));
    REQUIRE(!aCCb::str2num("2", valBool));      // only "0" and "1" are valid
    REQUIRE(!aCCb::str2num("false", valBool));  // only "0" and "1" are valid
    REQUIRE(!aCCb::str2num("FALSE", valBool));  // only "0" and "1" are valid
    REQUIRE(!aCCb::str2num("TRUE", valBool));   // only "0" and "1" are valid
    float valFloat;
    REQUIRE(aCCb::str2num("1.2345", valFloat) && (fabs(valFloat - 1.2345) < 1e-7));
    REQUIRE(aCCb::str2num("-1.2345", valFloat) && (fabs(valFloat + 1.2345) < 1e-7));
    double valDouble;
    REQUIRE(aCCb::str2num("1.2345", valDouble) && (fabs(valDouble - 1.2345) < 1e-17));
    REQUIRE(aCCb::str2num("-1.2345", valDouble) && (fabs(valDouble + 1.2345) < 1e-17));
}

TEST_END
