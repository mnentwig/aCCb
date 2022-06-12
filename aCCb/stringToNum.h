#pragma once
#include <cassert>
#include <stdexcept>
#include <string>
namespace aCCb {
using std::string;
int stoi(string val) {
	size_t ix;
	int retval;
	try {
		retval = std::stoi(val, &ix);
	} catch (std::exception &e) {
		throw e; // throw from "foreign" binary code is not reliably caught in gdb => re-throw
	}
	while (ix < val.length()) {
		switch (val[ix]) {
		case ' ':
		case '\n':
		case '\r':
		case '\t':
		case '\v':
			++ix;
			continue;
		default:
			throw std::runtime_error("conversion string to number failed");
		}
	}
	return retval;
}
} // NS
