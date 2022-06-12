// repeated inclusion is intended e.g. to enable / disable profiling in parts of the code only
// any code that reports profiling info may #include profilerUser_ON or profilerUser_OFF, to enable or disable data collection.
#include "profiler.hpp"

// this file enables profiling by turning TIC() and TOC() in user code into profiler calls.
#undef TIC
#define TIC(a) gprof.tic(a);

#undef TOC
#define TOC(a) gprof.toc(a);

#undef DECLARE
#define DECLARE(a, b) gprof.declare(a, b);
