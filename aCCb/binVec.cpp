#pragma once
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <cassert>
#include <bitset>
namespace aCCb {
using std::vector;
class binVec {
public:
	binVec(size_t nBits) :
			data(/*nElem*/(nBits + 63) / 64, /*default value*/0), nBits(nBits) {
	}

	binVec() :
			binVec(0) {
	}

	/** binVec to std::vector<bool> (copy) */
	inline operator vector<bool>&() const {
		vector<bool> r(/*nElem*/this->nBits, /*default value*/false);
		size_t ixMax = this->nBits;
		for (size_t ix = 0; ix < ixMax; ++ix)
			if (this->get(ix))
				r[ix] = true;
		return r;
	}

	/** std::vector<bool> to binVec (copy) */
	binVec(const vector<bool> inp) :
			binVec(inp.size()) {
		for (size_t ix = 0; ix < inp.size(); ++ix)
			if (inp[ix])
				this->setTrue(ix);
	}

	/** number of bits */
	size_t size() const {
		return this->nBits;
	}

	//* gets bit at position ix */
	inline bool get(size_t ix) const {
		assert(ix < this->nBits);
		return (this->data[ix >> 6] & ((uint64_t) 1 << (ix & 0x3F))) != 0;
	}

	//* sets bit at position ix to true */
	inline void setTrue(size_t ix) {
		assert(ix < this->nBits);
		this->data[ix >> 6] |= ((uint64_t) 1 << (ix & 0x3F));
	}

	//* sets bit at position ix to false */
	inline void setFalse(size_t ix) {
		assert(ix < this->nBits);
		this->data[ix >> 6] &= ~((uint64_t) 1 << (ix & 0x3F));
	}

	//* sets bit at position ix to v */
	inline void setFalse(size_t ix, bool v) {
		if (v)
			this->setTrue(ix);
		else
			this->setFalse(ix);
	}

	//* OR operator. Empty vector: identity element (all-0) */
	inline binVec operator |(const binVec &arg) const {
		// empty vector => identity element
		if (this->nBits == 0)
			return binVec(arg);
		if (arg.nBits == 0)
			return binVec(*this);

		assert(arg.nBits == this->nBits);
		binVec r(this->nBits);
		size_t ixMax = this->data.size();
		for (size_t ix = 0; ix < ixMax; ++ix)
			r.data[ix] = this->data[ix] | arg.data[ix];
		return r;
	}

	//* AND operator. Empty vector: identity element (all-0) */
	inline binVec operator &(const binVec &arg) const {
		// empty vector => identity element
		if (this->nBits == 0)
			return binVec(arg);
		if (arg.nBits == 0)
			return binVec(*this);

		assert(arg.nBits == this->nBits);
		binVec r(this->nBits);
		size_t ixMax = this->data.size();
		for (size_t ix = 0; ix < ixMax; ++ix)
			r.data[ix] = this->data[ix] & arg.data[ix];
		return r;
	}

	//* XOR operator. Empty vector: identity element ('not' the other argument) */
	inline binVec operator ^(const binVec &arg) const {
		// empty vector => identity element
		if (this->nBits == 0)
			return binVec(arg);
		if (arg.nBits == 0)
			return binVec(*this);

		assert(arg.nBits == this->nBits);
		binVec r(this->nBits);
		size_t ixMax = this->data.size();
		for (size_t ix = 0; ix < ixMax; ++ix)
			r.data[ix] = this->data[ix] ^ arg.data[ix];
		return r;
	}

	//* NOT operator */
	inline binVec operator !() {
		binVec r(this->nBits);
		size_t ixMax = this->data.size();
		for (size_t ix = 0; ix < ixMax; ++ix)
			this->data[ix] = ~this->data[ix];
		return this;
	}

	//* in-place OR operator. Empty vector: identity element (all-0) */
	inline void operator |=(const binVec &arg) {
		if (arg.nBits == 0)
			return;
		if (this->nBits == 0) {
			*this = arg;
			return;
		}
		assert(this->nBits == arg.nBits);
		size_t ixMax = this->data.size();
		for (size_t ix = 0; ix < ixMax; ++ix)
			this->data[ix] |= arg.data[ix];
	}

	//* in-place AND operator. Empty vector: identity element (all-1) */
	inline void operator &=(const binVec &arg) {
		if (arg.nBits == 0)
			return;
		if (this->nBits == 0) {
			*this = arg;
			return;
		}
		assert(this->nBits == arg.nBits);
		size_t ixMax = this->data.size();
		for (size_t ix = 0; ix < ixMax; ++ix)
			this->data[ix] &= arg.data[ix];
	}

	//* in-place XOR operator. Empty vector: identity element ('not' the other argument) */
	inline void operator ^=(const binVec &arg) {
		if (arg.nBits == 0)
			return;
		if (this->nBits == 0) {
			*this = arg;
			return;
		}
		assert(this->nBits == arg.nBits);
		const size_t ixMax = this->data.size();
		for (size_t ix = 0; ix < ixMax; ++ix)
			this->data[ix] ^= arg.data[ix];
	}

	/** clears bits that are true in arg */
	inline void clearBits(const binVec &arg) {
		if (this->nBits == 0) { // identity element in 'this': Assume all-ones
			*this = !arg;
			return;
		}
		if (arg.nBits == 0) { // identity element in arg: clear nothing
			return;
		}
		assert(this->nBits == arg.nBits);
		const size_t ixMax = this->data.size();
		for (size_t ix = 0; ix < ixMax; ++ix)
			this->data[ix] &= ~arg.data[ix];
	}

	//* count number of set bits */
	size_t popcount() const {
		size_t r = 0;
		const size_t ixMax = this->data.size();
		for (size_t ix = 0; ix < ixMax; ++ix)
			r += std::bitset<64>(this->data[ix]).count();
		return r;
	}

protected:
	vector<uint64_t> data;
	size_t nBits;
};
} // NS
