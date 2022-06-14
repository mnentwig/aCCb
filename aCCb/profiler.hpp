#pragma once
#include "profilerUser_ON.h" // enable by default
#include <set>
#include <map>
#include <stack>
#include <string>
#include <stdexcept>
#include <chrono>
#include <string>
#include <algorithm>
namespace aCCb {
using std::map;
using std::string;
using std::stack;
using std::set;
using std::runtime_error;
using std::string;
class profiler {
	typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_t;
	class entry {
	public:
		entry(int key, timepoint_t timestamp) :
				key(key), timestamp(timestamp) {
		}
		int key;
		timepoint_t timestamp;
	};
public:
	profiler() :
			ticAcc(), ticStack(), keyNames() {
	}
	void tic(int key) {
		timepoint_t tNow = std::chrono::high_resolution_clock::now();
		this->ticStack.emplace(key, tNow);
		if (this->ticStack.size() > 1000)
			throw runtime_error("too many levels of recursion: tic() without toc()");
	}
	void toc(int key) {
		timepoint_t tNow = std::chrono::high_resolution_clock::now();
		if (this->ticStack.size() == 0)
			throw runtime_error("toc() without tic()");
		entry &prev = this->ticStack.top();
		if (prev.key != key)
			throw runtime_error("toc() key does not match tic()");
		int64_t duration = std::chrono::duration_cast<std::chrono::nanoseconds>(tNow - prev.timestamp).count();
		this->ticAcc[key] += duration;
		this->ticStack.pop();
	}
	string report() {
		if (this->ticStack.size() != 0)
			throw runtime_error("tic() without toc()");
		string r = "";
		for (auto e : this->ticAcc) {
			int key = e.first;
			double t_us = (double) e.second / 1000.0;
			double t_ms = (double) e.second / 1000000.0;

			if (this->keyNames.find(key) == this->keyNames.end())
				throw runtime_error("undeclared key: " + std::to_string(key));
			string keyName = this->keyNames[key];
			r += keyName + "\t" + std::to_string(t_us) + " us" + "\t" + std::to_string(t_ms) + " ms\n";
		}
		this->ticAcc.clear();
		return r;
	}

	void declare(int key, const string keyName) {
		auto it = this->keyNames.find(key);
		if (it != this->keyNames.end())
			if (it->second != keyName)
				throw runtime_error("key description on redeclaration does not match");
		this->keyNames[key] = keyName;
	}
protected:
	map<int, int64_t> ticAcc;
	stack<entry> ticStack;
	map<int, string> keyNames;
};
} // NS

// for an executable with profiling, the global object needs to be instantiated exactly once.
// E.g. keep "aCCb::profiler gprof;" with main() (or wherever), afer #including this file.
// the "extern" will be reset by the later definition
extern aCCb::profiler gprof;
