#pragma once
#include <filesystem>
#include <string>
#include <unordered_set>
#include <regex>

namespace aCCb {
using std::string;
using std::unordered_set;
using std::regex;
static unordered_set<string> getFilesInDirectory(string path, regex pattern, bool includeFiles, bool includeDirs, bool keepPath) {
	unordered_set<string> files;

	regex r(pattern);
	for (const auto &file : std::filesystem::directory_iterator(path)) {
		if (file.is_regular_file() && !includeFiles)
			continue;
		if (file.is_directory() && !includeDirs)
			continue;

		string s = keepPath ? file.path().string() : file.path().filename().string();
		if (std::regex_match(s, pattern))
			files.insert(s);
	}
	return files;
}
} // NS
