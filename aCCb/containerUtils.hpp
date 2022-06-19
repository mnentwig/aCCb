#include <algorithm>
#pragma once
namespace aCCb::containerUtils {
/** check whether @param container contains @elem (via std::find, readability improvement) */
template<class T1, class T2> inline bool contains(const T1 &container, T2& val) {
	return std::find(container.cbegin(), container.cend(), val) != container.cend();
}

/** insert @param elem into @param container at position @param pos, possibly padding with default elements */
template<class T1, class T2> inline void putAt(T1& container, T2& elem, size_t pos) {
	if (container.size() < pos) {
		container.resize(pos);
	}
	if (container.size() == pos) {
		container.push_back(elem);
	} else {
		container[pos] = elem;
	}
}

/** insert @param elem into @param container at position @param pos, possibly padding with default elements */
template<class T1, class T2> inline void putAtVal(T1& container, T2 elem, size_t pos) {
	if (container.size() < pos) {
		container.resize(pos);
	}
	if (container.size() == pos) {
		container.push_back(elem);
	} else {
		container[pos] = elem;
	}
}

} // NS: aCCb::containerUtils
