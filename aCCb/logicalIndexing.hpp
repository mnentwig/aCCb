#pragma once
#include <vector>
#include <unordered_map>
#include <string>
namespace aCCb {
using std::vector;
using std::unordered_map;
using std::string;

namespace logicalIndexing {
namespace details {

/** returns number of true elements in indexOp */
size_t popcount(const vector<bool> &indexOp) {
	return std::count(indexOp.cbegin(), indexOp.cend(), true); // should ideally use popcnt intrinsic (reportedly, the C++ 20 implementation does)
}

/** picks data elements identified by indexOp, knowing the number of output elements */
template<class T> vector<T> applyIndexKnownPopcount_vector(const vector<T> &data, const vector<bool> &indexOp, size_t popcount) {
	vector<T> retVal;
	assert(data.size() == indexOp.size());
	retVal.reserve(popcount);
	auto itData = data.cbegin();
	for (auto itIndex = indexOp.cbegin(); itIndex != indexOp.cend(); ++itIndex, ++itData)
		if (*itIndex)
			retVal.push_back(*itData);
	return retVal;
}

/** picks data elements identified by indexOp in all vectors of the map, knowing the number of output elements */
template<class T, class tKey> unordered_map<tKey, vector<T>> applyIndexKnownPopcount_map(const unordered_map<tKey, vector<T>> &data, const vector<bool> &indexOp, size_t popcount) {
	unordered_map<tKey, vector<T>> retVal;
	for (auto itData : data)
		retVal[itData.first] = logicalIndexing::details::applyIndexKnownPopcount_vector<T>(itData.second, indexOp, popcount);
	return retVal;
}

/** helper function: data map lookup with runtime error if nonexisting field */
template<class T> vector<T>& getFieldOrThrow(unordered_map<string, vector<T>> &data, const string key) {
	auto it = data.find(key);
	if (it == data.end())
		throw runtime_error("field does not exist");
	return it->second;
}

} // NS: logicalIndexing::details

// =======================================================================================

/** picks data elements identified by indexOp, knowing the number of true elements */
template<class T> vector<T> applyIndex(const vector<T> &data, const vector<bool> &indexOp) {
	return details::applyIndexKnownPopcount_vector<T>(data, indexOp, details::popcount(indexOp));
}

/** generates logical indexing vector by applying expr on data */
template<class T> vector<bool> generateIndex(const vector<T> data, bool (*expr)(const T&)) {
	vector<bool> retVal;
	retVal.reserve(data.size());
	for (auto d : data)
		retVal.push_back(expr(d));
	return retVal;
}

/** picks data elements identified by indexOp for each value of the map*/
template<class T, class tKey> unordered_map<tKey, vector<T>> applyIndexMap(const unordered_map<tKey, vector<T>> &data, const vector<bool> &indexOp) {
	size_t popcount = details::popcount(indexOp);
	unordered_map<tKey, vector<T>> retVal;
	assert(data.size() != indexOp.size());
	for (auto itData : data)
		retVal[itData.first] = details::applyIndexKnownPopcount_map(itData.second, indexOp, popcount);
	return retVal;
}

// =======================================================================================

/** Container for vector maps with commonly used data types */
class vecMap;
/** proxy class to emulate get(string key) overloading on return type */
class vecMapExtractor {
public:
	vecMapExtractor(vecMap &vm, const string key) :
			vm(vm), key(key) {
	}
	operator vector<float>&() const;
	operator vector<double>&() const;
	operator vector<bool>&() const;
	operator vector<string>&() const;
	operator vector<uint8_t>&() const;
	operator vector<int8_t>&() const;
	operator vector<uint16_t>&() const;
	operator vector<int16_t>&() const;
	operator vector<uint32_t>&() const;
	operator vector<int32_t>&() const;
	operator vector<uint64_t>&() const;
	operator vector<int64_t>&() const;
protected:
	vecMap &vm;
	const string key;
};

class vecMap {
	friend vecMapExtractor;
public:
	vecMap() :
			nElem(0), initialized(false), data_float(), data_double(), data_bool(), data_string(), data_uint8(), data_int8(), data_uint16(), data_int16(), data_uint32(), data_int32(), data_uint64(), data_int64() {
	}

	/** picks data elements identified by indexOp for all vectors in all maps */
	vecMap applyIndex(vector<bool> &indexOp) {
		vecMap retVal;
		retVal.nElem = details::popcount(indexOp);
		retVal.data_float = details::applyIndexKnownPopcount_map(this->data_float, indexOp, retVal.nElem);
		retVal.data_double = details::applyIndexKnownPopcount_map(this->data_double, indexOp, retVal.nElem);
		retVal.data_bool = details::applyIndexKnownPopcount_map(this->data_bool, indexOp, retVal.nElem);
		retVal.data_string = details::applyIndexKnownPopcount_map(this->data_string, indexOp, retVal.nElem);
		retVal.data_uint8 = details::applyIndexKnownPopcount_map(this->data_uint8, indexOp, retVal.nElem);
		retVal.data_int8 = details::applyIndexKnownPopcount_map(this->data_int8, indexOp, retVal.nElem);
		retVal.data_uint8 = details::applyIndexKnownPopcount_map(this->data_uint8, indexOp, retVal.nElem);
		retVal.data_int8 = details::applyIndexKnownPopcount_map(this->data_int8, indexOp, retVal.nElem);
		retVal.data_uint8 = details::applyIndexKnownPopcount_map(this->data_uint8, indexOp, retVal.nElem);
		retVal.data_int8 = details::applyIndexKnownPopcount_map(this->data_int8, indexOp, retVal.nElem);
		retVal.data_uint8 = details::applyIndexKnownPopcount_map(this->data_uint8, indexOp, retVal.nElem);
		retVal.data_int8 = details::applyIndexKnownPopcount_map(this->data_int8, indexOp, retVal.nElem);
		return retVal;
	}

	/** get vector for key.
	 * Type is deduced from return type ("overload by return type" via proxy object).
	 * Use e.g. as vector<int64_t>& myVec = myVecMap.get("myKey");
	 * An explicit return type is mandatory. "auto" is not possible.
	 * Note, the type must be the same one when the data was "set()" or the element will not be found.*/
	vecMapExtractor get(const string key) {
		return vecMapExtractor(*this, key);
	}

	// === set ===
	void set(const string key, vector<float> &data) {
		this->initializeCheck(data.size());
		this->data_float[key] = data;
	}

	void set(const string key, vector<double> &data) {
		this->initializeCheck(data.size());
		this->data_double[key] = data;
	}

	void set(const string key, vector<bool> &data) {
		this->initializeCheck(data.size());
		this->data_bool[key] = data;
	}

	void set(const string key, vector<string> &data) {
		this->initializeCheck(data.size());
		this->data_string[key] = data;
	}

	void set(const string key, vector<uint8_t> &data) {
		this->initializeCheck(data.size());
		this->data_uint8[key] = data;
	}

	void set(const string key, vector<int8_t> &data) {
		this->initializeCheck(data.size());
		this->data_int8[key] = data;
	}

	void set(const string key, vector<uint16_t> &data) {
		this->initializeCheck(data.size());
		this->data_uint16[key] = data;
	}

	void set(const string key, vector<int16_t> &data) {
		this->initializeCheck(data.size());
		this->data_int16[key] = data;
	}

	void set(const string key, vector<uint32_t> &data) {
		this->initializeCheck(data.size());
		this->data_uint32[key] = data;
	}

	void set(const string key, vector<int32_t> &data) {
		this->initializeCheck(data.size());
		this->data_int32[key] = data;
	}

	void set(const string key, vector<uint64_t> &data) {
		this->initializeCheck(data.size());
		this->data_uint64[key] = data;
	}

	void set(const string key, vector<int64_t> &data) {
		this->initializeCheck(data.size());
		this->data_int64[key] = data;
	}

// === get ===
protected:
	/** helper function: runtime check enforces equal length of all contained vectors */
	void initializeCheck(size_t newSize) {
		if (!this->initialized) {
			this->initialized = true;
			this->nElem = newSize;
		} else if (newSize != this->nElem) {
			throw runtime_error("size mismatch");
		}
	}

	/** common length of all vectors (once initialized) */
	size_t nElem;

	/** whether at least one vector was set, making the length known */
	bool initialized;
	unordered_map<string, vector<float>> data_float;
	unordered_map<string, vector<double>> data_double;
	unordered_map<string, vector<bool>> data_bool;
	unordered_map<string, vector<string>> data_string;
	unordered_map<string, vector<uint8_t>> data_uint8;
	unordered_map<string, vector<int8_t>> data_int8;
	unordered_map<string, vector<uint16_t>> data_uint16;
	unordered_map<string, vector<int16_t>> data_int16;
	unordered_map<string, vector<uint32_t>> data_uint32;
	unordered_map<string, vector<int32_t>> data_int32;
	unordered_map<string, vector<uint64_t>> data_uint64;
	unordered_map<string, vector<int64_t>> data_int64;
};

inline vecMapExtractor::operator vector<float>&() const {
	return details::getFieldOrThrow(this->vm.data_float, this->key);
}

inline vecMapExtractor::operator vector<double>&() const {
	return details::getFieldOrThrow(this->vm.data_double, this->key);
}

inline vecMapExtractor::operator vector<bool>&() const {
	return details::getFieldOrThrow(this->vm.data_bool, this->key);
}

inline vecMapExtractor::operator vector<string>&() const {
	return details::getFieldOrThrow(this->vm.data_string, this->key);
}

inline vecMapExtractor::operator vector<uint8_t>&() const {
	return details::getFieldOrThrow(this->vm.data_uint8, this->key);
}

inline vecMapExtractor::operator vector<int8_t>&() const {
	return details::getFieldOrThrow(this->vm.data_int8, this->key);
}

inline vecMapExtractor::operator vector<uint16_t>&() const {
	return details::getFieldOrThrow(this->vm.data_uint16, this->key);
}

inline vecMapExtractor::operator vector<int16_t>&() const {
	return details::getFieldOrThrow(this->vm.data_int16, this->key);
}

inline vecMapExtractor::operator vector<uint32_t>&() const {
	return details::getFieldOrThrow(this->vm.data_uint32, this->key);
}

inline vecMapExtractor::operator vector<int32_t>&() const {
	return details::getFieldOrThrow(this->vm.data_int32, this->key);
}

inline vecMapExtractor::operator vector<uint64_t>&() const {
	return details::getFieldOrThrow(this->vm.data_uint64, this->key);
}
inline vecMapExtractor::operator vector<int64_t>&() const {
	return details::getFieldOrThrow(this->vm.data_int64, this->key);
}

} // NS: logicalIndexing
} // NS: aCCb
