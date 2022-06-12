#include "bnDataset.h"

#include "../aCCb/fileToString.hpp"
#include "../aCCb/getFilesInDirectory.hpp"
#include "../aCCb/profiler.hpp"
#include "../aCCb/splitToLines.hpp"
#include "../aCCb/stringToNum.hpp"

int objCppExampleForMakefile() {
	return 42;
}

template<class T> void vecappend(T a, const T b) {
	a.insert(a.end(), b.cbegin(), b.cend());
}

void bnDataset::append(const bnDataset &other) {
	vecappend(this->name, other.name);
	vecappend(this->mf, other.mf);
	vecappend(this->count, other.count);
}

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

static bnDataset loadOneFile(const string filename) {
	bnDataset retVal;

	int year = getYear(filename);

	// === load whole text file into string ===
	string contents = aCCb::fileToString(filename);

	// === split to lines ===
	vector<string> lines = aCCb::splitToLines(contents);

	// === remove dummy line (CR is terminator, not separator) ===
	aCCb::removeLastDummyLine(lines);
	DECLARE(1000, "process");
	TIC(1000);
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
	TOC(1000);
	return retVal;
}

bnDataset::bnDataset() {
}

bnDataset::bnDataset(const string directory) {
	const regex r("^.*txt$");
	unordered_set<string> files = aCCb::getFilesInDirectory("sampledataHistoricalBabynames",
	/* regex */r,
	/* include files */true,
	/* include directories */false,
	/* keep path in results */true);

	DECLARE(1001, "load");
	DECLARE(1002, "append");

	for (auto it : files) {
		cout << it << "\n";
		TIC(1001);
		bnDataset dFile = loadOneFile(it);
		TOC(1001);
		TIC(1002);
		this->append(dFile);
		TOC(1002);
	}
}
