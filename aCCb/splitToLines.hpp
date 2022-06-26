#pragma once
#include <cassert>
#include <vector>
#include <string>
#include <regex>
#include <stdexcept>
namespace aCCb {
using std::vector;
using std::string;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static vector<string> splitToLines_STL(const string contents) {
	vector<string> retval;
	const regex rSplit(R"((.*)\u000D?\u000A)"); //
	std::smatch m;
	string::const_iterator searchStart(contents.cbegin());
	string::const_iterator searchEnd(contents.cend());
	while (searchStart != searchEnd) {
		std::regex_search(searchStart, searchEnd, m, rSplit);
		assert(m.size() == 2);
		retval.emplace(retval.end(), m[1]);
		searchStart = m.suffix().first;
	}

	// always return the trailing end after the final line break.
	// If line break was used as a separator, there will be an extra element.
	// Handling this is left to the caller.
	retval.push_back(string(searchStart, searchEnd));
	return retval;
}
#pragma GCC diagnostic pop

inline std::vector<std::string> splitRegex(const std::string str, const std::string regex_str) {
	std::regex r(regex_str); // cannot use a temporary expression (one-liner)
	return {std::sregex_token_iterator(str.begin(), str.end(), r, -1), /*equiv. to end()*/std::sregex_token_iterator()};
}

inline vector<string> splitToLines(const string contents) {
	vector<string> retval;

	size_t start = 0;
	char cPrev = 0; // startup: '\0 is not \r'
	size_t ix;
	for (ix = 0; ix < contents.length(); ++ix) {
		char c = contents[ix];
		if (c == '\n') {
			size_t end = ix;
			if (cPrev == '\r')
				--end;
			std::string_view v(&contents[start], end - start);
			retval.emplace(retval.end(), v);
			start = ix + 1;
		}
		cPrev = c;
	}
	assert(ix == contents.length());
	assert(start <= ix);
	std::string_view v(&contents[start], ix - start);
	retval.emplace(retval.end(), v);
	return retval;
}

static void removeLastDummyLine(vector<string> &vec) {
	if (vec.size() == 0)
		throw std::runtime_error("got empty vector");
	string dummyLine = vec.back();
	if (dummyLine.length() != 0)
		throw std::runtime_error("last line not empty");
	vec.pop_back();
}
} // NS
