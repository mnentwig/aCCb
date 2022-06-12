# aCppCookbook
## Motivation
I like modern (STL) C++ but don't use it frequently enough to remember everything.

Looking the same things up again and again on Stackexchange gets tiresome, so I decided to collect the "basic building blocks" for copy-and-paste reuse. This selection is completely arbitrary, with no claims to completeness.

Consider this "notes-to-self" that may or may not make much sense to someone else.

## Eclipse
* Eclipse*s own build system makes it cumbersome to maintain a large number of executables in one project (need to manually set file exclusion for every toplevel .cpp file after adding a new toplevel file).

* A handwritten makefile seems more straightforward but needs to take into account that Eclipse will call it from the build folder with e.g. "make -f ../../Makefile".

* Eclipse's generated sample makefile uses "PROJECT_ROOT = .." at the beginning, which isn't robust when the makefile gets more complex. Use ":=" instead of "=" for immediate (not delayed) expansion of the variable.

* The keyboard shortcuts for "Run (F11)" and "Debug (Ctrl-F11)" change the selected "launch configuration, usually resetting it to the previously launched value. To change it, use the "Launch" toolbar button on the left or select from the drop-down list "Run xyz" in the "Run" toolbar button.

* The Eclipse console seems unreliable: In some cases, output to stderr immediately before exit() was missing despite being flushed with std::endl.

* Breakpoint on thrown exceptions: Debug view, "..." button ("View menu"), "Add event breakpoint (C/C++)", select "exception thrown". Some exceptions e.g. from "std::stoi" are not caught (but "catch throw" to gdb in the "Debugger console" gives the same result. Wrapping 

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

## Performance observations
* not initializing a smatch before the regex had a measurable performance impact.
* for vector<string>, the performance benefit of .emplace vs .push_back is clearly visible