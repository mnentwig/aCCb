#pragma once
#include <map>
#include <stack>
#include <string>
#include <unordered_set>
#include <stdexcept>
#include <chrono>
#include <string>
namespace aCCb {
using std::map;
using std::string;
using std::stack;
using std::unordered_set;
using std::runtime_error;
using std::string;
class profiler {
typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_t;
	class entry {
	public:
		entry(string key, timepoint_t timestamp) :
				key(key), timestamp(timestamp) {
		}
		string key;
		timepoint_t timestamp;
	};
public:
	profiler() {
	}
	void tic(const string key) {
		timepoint_t tNow = std::chrono::high_resolution_clock::now();
		if (this->openKeys.find(key) != this->openKeys.end())
			throw runtime_error("recursion");
		this->openKeys.insert(key);
		this->ticStack.emplace(key, tNow);
	}
	void toc() {
		timepoint_t tNow = std::chrono::high_resolution_clock::now();
		if (this->ticStack.size() == 0)
			throw runtime_error("toc() without tic()");
		entry &prev = this->ticStack.top();
		int64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(tNow - prev.timestamp).count();
		this->ticAcc[prev.key] += duration;
		this->openKeys.erase(prev.key);
		this->ticStack.pop();
	}
	string report() {
		if (this->openKeys.size() != 0)
			throw runtime_error("tic() without toc()");
		string r = "";
		for (auto e : this->ticAcc) {
			r += e.first + "\t" + std::to_string((double) e.second / 1000.0) + "\n";
		}
		this->ticAcc.clear();
		return r;
	}
protected:
	map<const string, int64_t> ticAcc;
	set<string> openKeys;
	stack<entry> ticStack;
};
} // NS
