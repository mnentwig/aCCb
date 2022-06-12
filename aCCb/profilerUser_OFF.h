// repeated inclusion is intended e.g. to enable / disable profiling in parts of the code only
// any code that reports profiling info may #include profilerUser_ON or profilerUser_OFF, to enable or disable data collection.

// this file disables profiling by ignoring TIC() and TOC() in user code
#undef TIC
#define TIC()
#undef TOC
#define TOC()
#undef DECLARE
#define DECLARE()
