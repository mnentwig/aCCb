#pragma once

#if defined DOCTEST_VERSION_MAJOR || defined CATCH_VERSION_MAJOR
// other framework present? We need those two to generate main(), do nothing in other frameworks
#	define TEST_START
#	define TEST_END
#else
//#ifdef NDEBUG
//#	error "NDEBUG may not be set, unitTest inserts assert()"
//#endif
#include <iostream>
// main function opens
#define TEST_START void main2() {
// main function closes
#define TEST_END 															\
} /* void main2() */														\
int main() {																\
	try {																	\
		main2();															\
	} catch (std::exception &e) {											\
		std::cerr << "exception:" << e.what() << "\n" << std::flush;		\
		return 1;															\
	}																		\
	std::cout << "all unit tests passed\n" << std::flush;					\
	return /*EXIT_SUCCESS*/0;												\
} /* int main() */

// === standard feature TEST_CASE ===
// (do nothing, must be wrapped byu TEST_START ... TEST_END
#define TEST_CASE(a)
// === standard feature REQUIRE ===
#define REQUIRE(a) assert(a)
// standard feature === CHECK_THROWS_WITH ===
#define CHECK_THROWS_WITH(a,b) 												\
	try {																	\
	a; 																		\
		throw std::runtime_error("no exception!");							\
	} catch (std::exception &e){ 											\
	if (std::string(e.what()).compare(b)){ 									\
		std::cerr << "expected " << b << " got " << e.what() << std::endl; 	\
		exit(1); 															\
	} 																		\
}
#endif // if other framework present
