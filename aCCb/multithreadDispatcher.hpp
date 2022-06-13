#pragma once

//#define USE_BSTP
#ifndef USE_BSTP
#	include <thread>
#	include <future>
#else
#	include "../3rdParty/BS_Thread_Pool.hpp"
#endif

#include <vector>

namespace aCCb {
using std::vector;
template<typename Tin, typename Tout>
static vector<Tout> multithreadDispatcher(vector<Tin> &inputs, Tout (*workFun)(Tin arg)) {
	vector<Tout> retVal;
	unsigned int n = inputs.size();
	retVal.reserve(n);
	if (n <= 1) {
		// === reference implementation ===
		for (Tin &inp : inputs)
			retVal.push_back(workFun(inp));
	} else {
		typedef std::future<Tout> future_t;
		vector<future_t> futures;
		futures.reserve(inputs.size());
#ifdef USE_BSTP
		BS::thread_pool pool;
		for (unsigned int ix = 0; ix < n; ++ix)
			futures.push_back(pool.submit(workFun, inputs[ix]));
#else
		for (unsigned int ix = 0; ix < n; ++ix)
			futures.push_back(std::async(workFun, inputs[ix]));
#endif
		for (unsigned int ix = 0; ix < n; ++ix)
			retVal.push_back(futures[ix].get());
	}
	return retVal;
}
} // NS
