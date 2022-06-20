#include <iostream>
#include <sstream>
#include "../aCCb/csvTable.hpp"
#include "../aCCb/stringToNum.hpp"

int mainB() {

	int16_t val;
	if (aCCb::str2num(string("32767"), val))
		std::cout << val << "\n" << std::flush;
	exit(0);

	std::istringstream buffer("\t 65535\t  \r\n        _");
	uint16_t var;
	string var2;
	buffer >> var;
	if (!buffer.good()) {
		std::cout << "numconv fail\n" << std::flush;
	} else {
		buffer >> var2;
		if (buffer.good())
			std::cout << "excess data\n" << std::flush;
		if (var2 != "_")
			std::cout << "trailing data\n" << std::flush;
		std::cout << var << "\n" << std::flush;
	}
	return 0;
	//std::istringstream csv("one,two,three\n\r\"four, four\nmore four\",five,six\nseven,eight,\"nine\"\r\n");
	std::istringstream csv("1,2,3\n4,5,6\n7,8,9");
	tableFactory::importSpec s;
	s.registerColumn(0, "colOne", s.colType_STRING);
	s.registerColumn(1, "colTwo", s.colType_UINT8);
	s.registerColumn(2, "colThree", s.colType_INT64);
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
