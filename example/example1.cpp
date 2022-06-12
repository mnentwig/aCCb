#include <iostream> // cout
using std::cout;
using std::endl;
#include <stdexcept> // runtime_error
#include <cassert>
#include <set>

#include "../aCCb/profiler.hpp"
aCCb::profiler p;
#define TIC(a) p.tic(a);
#define TOC() p.toc();

#include "../aCCb/getFilesInDirectory.h"
#include "../aCCb/fileToString.h"
#include "../aCCb/splitToLines.h"
#include "../aCCb/stringToNum.h"
#include "../aCCb/profiler.hpp"
using std::vector;
using std::set;
using std::unordered_set;
using std::string;
using std::regex;

template<class T> void vecappend(T a, const T b) {
	a.insert(a.end(), b.begin(), b.end());
}
struct myData {
	vector<string> name;
	vector<string> mf;
	vector<int> count;
	vector<int> year;
public:
	void append(struct myData &other) {
		vecappend(this->name, other.name);
		this->name.insert(this->name.begin(), other.name.begin(), other.name.end());
	}
};

int getYear(string filename) {
	// === extract the four-digit year from the filename ===
	string pat = "yob" // "year of birth"
					"("// capture starts
					R"(\d\d\d\d)"// four digits
					")"// capture ends
					R"(\.txt)"// .txt extension
					"$";// end of string
	const regex r(pat);
	std::smatch m;
	bool match = (std::regex_search(filename, m, r));
	if (!match)
		throw std::runtime_error("invalid filename: " + filename);

	// e.g. filename = "sampledataHistoricalBabynames\yob2017.txt"
	// m[0] contains the complete match "yob2017.txt"
	// m[1] contains the first capture "2017".
	assert(m.size() == 2);
	string res = m[1];
	assert(res.length() == 4);

	return aCCb::stoi(res);
}

myData loadOneFile(const string filename) {
	myData retVal;

	int year = getYear(filename);

	// === load whole text file into string ===
	string contents = aCCb::fileToString(filename);

	// === split to lines ===
	vector<string> lines = aCCb::splitToLines(contents);

	// === remove dummy line (CR is terminator, not separator) ===
	aCCb::removeLastDummyLine(lines);

	TIC("process");
	const string startOfLine = "^";
	const string notComma = "[^,]+";
	const string captNotComma = "(" + notComma + ")";
	const string comma = ",";
	const string endOfLine = "$";
	string pat2 = "" 					// (forces eclipse indentation, together with following /* */ comment)
	/*    */+ startOfLine 				//
			+ captNotComma + comma  	// name
			+ captNotComma + comma  	// gender
			+ captNotComma  			// count
			+ endOfLine;				//
	const regex r(pat2);

	std::smatch m;
	for (string line : lines) {
		bool match = (std::regex_search(line, m, r));
		if (!match)
			throw std::runtime_error("fail");
		assert(m.size() == 4);
		retVal.name.emplace(retVal.name.end(), m[1].str());
		retVal.mf.emplace(retVal.mf.end(), m[2]);
		retVal.count.emplace(retVal.count.end(), aCCb::stoi(m[3]));
		retVal.year.push_back(year);
	}
	TOC();
	return retVal;
}

bool myStrCmp(const string &s1, const string &s2) {
	return s1.compare(s2) > 0;
}

void example_setCustomComparer(struct myData &d) {

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
	auto itRevEnd = namesRev.crend();
	auto itRev2 = namesRev2.crbegin();
	auto itRev2End = namesRev2.crend();

	while (itForw != itForwEnd) {
		assert(itRev != itRevEnd);
		assert(itRev2 != itRev2End);
		string nForw = *itForw;
		string nRev = *itRev;
		string nRev2 = *itRev2;
		assert(nForw == nRev);
		assert(nForw == nRev2);
	}
}

int mainB() {
	cout << "current folder: " << std::filesystem::current_path() << endl;
	const regex r("^.*txt$");
	unordered_set<string> files = aCCb::getFilesInDirectory("sampledataHistoricalBabynames",
	/* regex */r,
	/* include files */true,
	/* include directories */false,
	/* keep path in results */true);

	struct myData d;
	for (auto it : files) {
		cout << it << "\n";
		TIC("load");
		struct myData dFile = loadOneFile(it);
		TOC();
		cout << p.report();
		return 0;
		cout << "appending" << endl << std::flush;
		d.append(dFile);
		cout << "done" << endl << std::flush;
	}

	example_setCustomComparer(d);
	cout << "Example done\n";
	return 0;
}

int main() {
	cout << "ex1\n";
	try {
		return mainB();
	} catch (std::exception &e) {
		std::cerr << "exception:" << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "unspecific exception" << std::endl;
	}
}
