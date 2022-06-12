#include <iostream>
#include <stdexcept>
#include "../aCCb/getFilesInDirectory.h"
using std::cout;
using std::unordered_set;
using std::string;
using std::regex;
int main2() {
	regex r("*.txt$");
	unordered_set<string> files = aCCb::getFilesInDirectory("sampleDataHistoricalBabynames", r, false, true, true);
	for (auto it : files)
		cout << it << "\n";
	return 0;
}

int main() {cout << "ex2\n";
	try {
		return main2();
	} catch (std::exception &e) {
		std::cerr << "exception:" << e.what() << "\n";
		return 1;
	}
}
