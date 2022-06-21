#pragma once
#include <cassert>
#include <stdexcept>
#include <string>
namespace aCCb {
using std::string;
inline int stoi(string val) {
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

template<typename T> inline bool str2num(const string &str, T &val) {
	std::stringstream ss; 		// note: uses "classic" C locale by default
	string sentry;
	string dummy;
	if (str.find('_') != str.npos)
		return false;			// string contains sentry character (which is never valid in a convertible number)
	ss << str << "_"; 			// append sentry character
	bool f1 = (ss >> val);		// read number, check whether successful
	bool f2 = (ss >> sentry);	// check for trailing characters (sentry: OK)
	return ((sentry == "_") && f1 && f2);
}

// special case as bytesize read from istream returns single character
template <> inline bool str2num(const string &str, uint8_t &val) {
	int val2;
	bool retval = str2num<int>(str, val2);
	val = (uint8_t) val2;
	return (retval && (val2 >= 0) && (val2 <= 0xFF));
}

// special case as bytesize read from istream returns single character
template <> inline bool str2num(const string &str, int8_t &val) {
	int val2;
	bool retval = str2num<int>(str, val2);
	val = (uint8_t) val2;
	return (retval && (val2 >= -128) && (val2 <= 127));
}

} // NS
