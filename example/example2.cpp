#include <algorithm> // next_permutation
#include <numeric> // iota
#include "../aCCb/stdIncludes.h"

#include "../aCCb/multithreadDispatcher.hpp"

// example worker function
int getNPerm(int nElem) {
	// throw runtime_error("testing the exception handling");
	vector<unsigned int> v(nElem);
	std::iota(v.begin(), v.end(), 0);
	int count = 0;
	while (std::next_permutation(v.begin(), v.end()))
		++count;
	return count;
}

int main2() {
	// === get work done in parallel ===
#if 0
	// === testcase: variable duration ===
	vector<int> v;
	for (int ix1 = 0; ix1 < 1000; ++ix1)
		for (int ix2 = 9; ix2 <= 11; ++ix2)
			v.push_back(ix2);
#elif 1
	// === testcase: uniform moderate duration ===
	vector<int> v;
	v.assign(/*num copies*/5000, /*value*/10);
#else
	// === testcase: extremely short duration ===
	vector<int> v(0);
	v.assign(/*num copies*/100000, /*value*/2);
#endif
	vector<int> res = aCCb::multithreadDispatcher(v, &getNPerm);

	// === print result ===
	if (res.size() > 100)
		cout << "num results: " << res.size() << "\n";
	else
		for (size_t ix = 0; ix < res.size(); ++ix)
			cout << ix << "\t" << res[ix] << "\n";
	return 0;
}

int main() {
	cout << "ex2\n" << std::flush;
	try {
		return main2();
	} catch (std::exception &e) {
		std::cerr << "exception:" << e.what() << "\n";
		return 1;
	}
}
