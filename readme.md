# aCppCookbook
## Motivation
Collection of reusable code snippets to reduce the amount of my future quick-and-dirty perl/Octave/
I like modern (STL) C++ but don't use it frequently enough to remember everything.

Looking the same things up again and again on Stackexchange gets tiresome, so I decided to collect the "basic building blocks" for copy-and-paste reuse. This selection is completely arbitrary, with no claims to completeness.

Consider this "notes-to-self" that may or may not make much sense to someone else.

## MinGW
* Always compile -static otherwise the executable may fail from a 'foreign' MSYS shell e.g. the one that comes with GIT

## Eclipse
* Eclipse*s own build system makes it cumbersome to maintain a large number of executables in one project (need to manually set file exclusion for every toplevel .cpp file after adding a new toplevel file).

* A handwritten makefile seems more straightforward but needs to take into account that Eclipse will call it from the build folder with e.g. "make -f ../../Makefile".

* Eclipse's generated sample makefile uses "PROJECT_ROOT = .." at the beginning, which isn't robust when the makefile gets more complex. Use ":=" instead of "=" for immediate (not delayed) expansion of the variable.

* The keyboard shortcuts for "Run (F11)" and "Debug (Ctrl-F11)" change the selected "launch configuration, usually resetting it to the previously launched value. To change it, use the "Launch" toolbar button on the left or select from the drop-down list "Run xyz" in the "Run" toolbar button.

* The Eclipse console is unreliable: E.g. sometimes final output before exit() is missing. Flushing does not help. 
* It may show in "run" mode when it is missing in "debug"

* Breakpoint on thrown exceptions: Debug view, "..." button ("View menu"), "Add event breakpoint (C/C++)", select "exception thrown". Some exceptions e.g. from "std::stoi" are not caught (but "catch throw" to gdb in the "Debugger console" gives the same result (unstandardized binary interface)? If needed, catch and re-throw from own code, then the debugger will detect it.

## VS Code
Use following setting for compact indentation:

<img src="doc/vsCode_setIndent.png">

That is, set C_Cpp.clang_format_fallbackStyle to { BasedOnStyle: Google, IndentWidth: 4, ColumnLimit: 0}

Press SHIFT-ALT-F to indent whole file.

F12: goto definition (CTRL-u: go back)

ALT-F12: peek at definition (ESC closes)

SHIFT-F12: references

F2: rename (all files, often not too useful)

CTRL-F2: Rename locally

SHIFT-CTRL-o: outline (to navigate quickly)

Switch editor left-/right: CTRL-pageup or -pagedown

Hide left (primary) side bar: CTRL-b

Hide bottom side bar: CTRL-j

Run main from .cpp file in current editor: Button at top right of screen ("Run" or "Debug" C++ file)

Shift-Ctrl-V: Open preview of Readme.md

## benchmark
A simple benchmark on splitting a whole text file:
* 110 ms STL/regex, debug
* 19 ms STL/regex, -O3 -DNDEBUG
* 6.5 ms "for loop", debug
* 1.7 ms "for loop" -O3 -DNDEBUG. String duplication via "emplace" accounts for 1.3 ms.

## Unit tests
* Option 1: "catch2" single-header framework. E.g. 40.4 s compile-/link time.
* Option 2: "doctest" single-header framework. E.g. 11.4 s
* Option 3: own minimal "aCCb" framework. E.g. 6.1 s 

## Notes: Performance
* not initializing a smatch before the regex had a measurable performance impact.
* for vector<string>, the performance benefit of .emplace vs .push_back is clearly visible

## Notes: Linker
* writing code in .hpp files instead of .h/.cpp is usually quicker to code
* design decision to replicate code instead of reusing some (non-template) shared object code 
* a standalone functions need to be marked 'static' or 'inline' to prevent "multiple definition" errors, as #including the header would by default export it from any object file.

## Notes: std::map
find an element:

```
auto it = this->keyNames.find(key);
if (it != this->keyNames.end())
	it->first / it->second; // key and value
```
## Notes: disable warning
```
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" 
// (or -function)
... code
#pragma GCC diagnostic pop
```

## Notes: std::vector
delete element k: This works, because vector has random-access iterators

```
vec.erase(vec.begin() + k);
```

vector from iterator: Use iterator-based constructor e.g. vector<...>(it.begin(), it.end()).

Can give arguments to constructor for return value in list e.g. 

```
vector<..> myFun(){ 
	return {it.begin(), it.end()};
}
```

## Notes: unordered
* std::iota(v.begin(), v.end(), startval) <numeric> fill with increasing numbers // iota refers to greek letter
* std::next_permutation(v.begin(), v.end()) <algorithm> swaps elements and returns true unless elements are sorted

## Notes: Multithreading
aCCb::MultithreadDispatcher shows a very straightforward multithreading pattern with std::async and std::future. 
A more sophisticated threadpool library is usually (but not always!) faster, see "BS_Thread_Pool.hpp" in "3rdParty".
* Both throw exceptions into the original thread at "get()" called on the future.
* The std::async pattern stalls at > 100k pending jobs. It is largely useless for short (sub-ms) jobs.
* A thread pool implementation (see 3rd party references) can be used even for short jobs and will give a notable speed increase.
* Note that std::async does not allow arguments to be passed by reference. Reportedly, std::ref() is dangerous.

## Notes: Simple use of async
This is largely useless for short (sub-ms) jobs, need to reuse threads (pool) instead.
```
#include <iostream>
#include <vector>
#include <future>

int testfun(int a, int b) {
	return a * b;
}

int main(int argc, char **argv) {
	std::vector<std::future<int>> futs;

	for (int ix = 0; ix < 1000; ++ix)
		futs.push_back(std::async(testfun, /*a*/ix, /*b*/ix + 1));

	for (int ix = 0; ix < futs.size(); ++ix)
		std::cout << futs[ix].get() << "\n";
}
```

## Useful 3rd party libraries (open source, single file)
* https://github.com/bshoshany/thread-pool Barak Shoshany, "A C++17 Thread Pool for High-Performance Scientific Computing"
* https://github.com/doctest Viktor Kirilov and contributors
* https://github.com/catchorg/Catch2 Martin Hořeňovský and contributors

## Notes: Warnings
* -Wall
* -Wfatal-errors: Stops at first error (console is easier to read)
* -Wextra 
* -Wpedantic  
* -Weffc++ 

## Notes: Memory model
* By default, variable reads and writes ('loads and stores') can be re-ordered arbitrarily
* std::atomic<supportedType> x; x.store(val) and x.load(val) prevents reordering across the instruction (fence/barrier)
* default 2nd arg std::memory_order_seq_cst is the most expensive

## Notes: language level
* C++11: __cplusplus is 201103L
* C++14: __cplusplus is 201402L
* C++17: __cplusplus is 201703L
* C++20: __cplusplus is 202002L

## Notes: Namespace
* Convention: Hide private functions in nested "details" namespace (or "Private" etc)
* Use descriptive, long namespace text, then create convenience alias e.g. namespace li = aCCb::logicalIndexing;
* method definitions should ideally not use aliases, as Eclipse shows the verbatim code in tooltips e.g. "auto" variable type

## Notes: Templates
* "Full" template specialization with template<> is broken (error: 'only at namespace scope'). 
However, implementing a conventional (non-template) function with the correct signature besides the template works.

* Overload on return type can be emulated by returning a proxy object that performs the switch by overloading its cast operator: "inline proxyClass::operator theTargetType() const {...}"

* Templates can fail weirdly e.g. functions with std::istream& arguments are not resolved. Use explicit types e.g. myFun<myTemplateType>(...)

## Notes: Default copy constructor
* delete when object/struct copy makes no sense e.g. holding file handles
* see "rule of 3/rule of 5"

## Notes: CRTP ("Curiously recuring template pattern")
Derived class T introduces an intermediate class with common code using template <class T> and "static_cast<T>(this). 
* More compact code
* avoids virtual calls 

## Notes: return types
* "auto" fills in complex return types: decltype(auto) myFun(){return someComplexType;}
* "auto" return type can be specified with -> syntax
* "decltype" determines type of an expression
* "declval" in such an expression provides access to objects and methods without requiring a constructor

## Notes: String to number
std::istringstream >> myVar works "almost" but fails silently on negative values for unsigned types. See str2num implementation.

## Notes: istream
* use return value e.g. bool flag = (is >> val); Caveats, see above.
* default locale is "classic" C

## Notes: Warnings
* Use -Wall -Wextra -Wconversion -Wsign-conversion -Wpedantic -Weffc++ -Wshadow

## Notes: Popcount
* Use std::bitset<myBitwidth>(myValue).count() or c++20 std::popcount
* __builtin_popcount() is 32 bit only! 

## Notes: "Structured bindings", "if initializer" (C++17)
```
if (auto [iter, succeeded] = mymap.insert(value); succeeded) {
    doSomethingWith(iter); // also available in "else" clause
}
```
## Notes: "Eigen"
* Use -arch=native flag
* Use -fopenmp
* -O(1) compiles 1/3 faster than -O0

## Notes: Templates
* if constexpr (...)
* e.g. std::is_unsigned_v<T> , std::is_integral_v<T>, , std::is_arithmetic_v<T> (has + - etc operators)
* can use e.g. int template parameter two switch between different variants of some code (using constexpr if)
* can use template specialization to add variant-dependent methods

## Notes: Float
In a float calculation, use the 'f' postfix diligently on constants. Otherwise, conversion to double may cause a significant performance penalty.

## Command line parser
See examples/cmdLineParser.cpp

## Notes: Polymorphism
Forget "virtual" => quality bug (upcasting to the base class then calling a method calls the method of the base class, not of the derived class)

## Notes: Address of element in STL container
The element can move in memory e.g. realloc to create more space. Check, under which conditions an iterator is invalidated.

## Notes: Profiling
```
auto begin = std::chrono::high_resolution_clock::now();
do_something()
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
cout << 1e6 * (double)duration << " ms" << endl;
```