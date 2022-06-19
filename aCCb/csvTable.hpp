#pragma once
#include <istream>
#include <string>
#include "../aCCb/logicalIndexing.hpp"
namespace li = aCCb::logicalIndexing;
using std::string;

class tableFactory {
public:
	static aCCb::logicalIndexing::vecMap csvTable(std::istream &is) {
		string line;
		while (!is.eof()) {
			std::getline(is, line);
			std::cout << "line:'" << line << "'\n" << std::flush;
		}
		std::cout << "done\n" << std::flush << std::endl;
		vecMapLoader self;
		return self;
	}
protected:
	class vecMapLoader: public li::vecMap {
	};
};
