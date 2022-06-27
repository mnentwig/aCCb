#pragma once
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <cassert>
namespace aCCb {
using std::vector;
class binVec {
public:
	binVec(size_t n) :
			n(n) {
		data = new uint64_t[this->getNWords()];
	}

	binVec(const vector<bool> inp) :
			binVec(inp.size()) {
		for (size_t ix = 0; ix < inp.size(); ++ix)
			this->put(ix, inp[ix]);
	}

	~binVec() {
		delete[] this->data;
	}

	/** cast to vector<bool> with same contents */
	inline operator vector<bool>&() const {
		vector<bool> r;
		r.reserve(this->n);
		for (size_t ix = 0; ix < this->getNWords(); ++ix)
			r.push_back(this->get(ix));
		return r;
	}

	//* gets bit at position ix */
	inline bool get(size_t ix) const {
		return (this->data[ix >> 6] & (1 << (ix & 0x3F))) != 0;
	}

	//* sets bit at position ix */
	inline void put(size_t ix, bool v) const {
		this->data[ix >> 6] |= (v << (ix & 0x3F));
	}

	inline binVec operator |(const binVec &arg) const {
		assert(this->n == arg.n);
		binVec r(this->n);
		for (size_t ix = 0; ix < this->getNWords(); ++ix)
			r.data[ix] = this->data[ix] | arg.data[ix];
		return r;
	}

	inline binVec operator &(const binVec &arg) const {
		assert(this->n == arg.n);
		binVec r(this->n);
		for (size_t ix = 0; ix < this->getNWords(); ++ix)
			r.data[ix] = this->data[ix] & arg.data[ix];
		return r;
	}

	inline binVec operator ^(const binVec &arg) const {
		assert(this->n == arg.n);
		binVec r(this->n);
		for (size_t ix = 0; ix < this->getNWords(); ++ix)
			r.data[ix] = this->data[ix] ^ arg.data[ix];
		return r;
	}

	inline binVec operator !(const binVec &arg) {
		binVec r(this->n);
		for (size_t ix = 0; ix < this->getNWords(); ++ix)
			this->data[ix] = ~arg.data[ix];
		return this;
	}

	inline binVec operator |=(const binVec &arg) {
		assert(this->n == arg.n);
		for (size_t ix = 0; ix < this->getNWords(); ++ix)
			this->data[ix] |= arg.data[ix];
		return this;
	}

	inline binVec operator &=(const binVec &arg) {
		assert(this->n == arg.n);
		for (size_t ix = 0; ix < this->getNWords(); ++ix)
			this->data[ix] &= arg.data[ix];
		return this;
	}

	inline binVec operator ^=(const binVec &arg) {
		assert(this->n == arg.n);
		for (size_t ix = 0; ix < this->getNWords(); ++ix)
			this->data[ix] ^= arg.data[ix];
		return this;
	}

protected:
	uint64_t *data;
	size_t n;
	inline size_t getNWords() const {
		return (this->n + 63) >> 6; // division by 64, rounding up
	}
};

}
// NS
