#pragma once
#include <fstream> // ifstream
#include <string>
namespace aCCb {
std::string fileToString(const std::string filename) {
	std::ostringstream all;
	all << std::ifstream(filename, std::ios::binary).rdbuf();
	return all.str();
}
} // NS
