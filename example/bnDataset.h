#pragma once
#include "../aCCb/stdIncludes.h"
#include "../aCCb/profilerUser_OFF.h"
class bnDataset {
public:
	bnDataset();
	bnDataset(const string directory);
	void append(const bnDataset &other);
	vector<string> name;
	vector<string> mf;
	vector<int> count;
	vector<int> year;
};
