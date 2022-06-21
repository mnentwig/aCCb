#include <iostream>
#include <sstream>
#include "../aCCb/csvTable.hpp"
#include "../aCCb/stringToNum.hpp"

int mainB() {

	//std::istringstream csv("one,two,three\n\r\"four, four\nmore four\",five,six\nseven,eight,\"nine\"\r\n");
	std::istringstream csv("1,2,3,4\n4,5,6, 7\n7,8,9,10");
	tableFactory::importSpec s;
	s.registerColumn(0, "colOne", s.colType_STRING);
	s.registerColumn(1, "colTwo", s.colType_UINT8);
	s.registerColumn(2, "colThree", s.colType_INT64);
	s.registerColumn(2, "colFour", s.colType_FLOAT);
	tableFactory::csvTable(csv, s);
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
