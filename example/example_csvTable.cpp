#include <iostream>
#include <sstream>
#include "../aCCb/csvTable.hpp"


int mainB() {
	std::istringstream csv("one,two,three\nfour,five,six\nseven,eight,nine");
	tableFactory::csvTable(csv);
	return 0;
}

int main() {

	try {
		return mainB();
	} catch (std::exception &e) {
		std::cerr << "exception:" << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "unspecific exception" << std::endl;
	}
}
