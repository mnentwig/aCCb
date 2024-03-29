# get the folder where this makefile is executed.
# Eclipse will run from a build folder, then run "make -f ../../Makefile"
# note the use of :=
# The expression needs to be "expanded" (evaluated) immediately, not when it is used.
# Otherwise, MAKEFILE_LIST may have been modified, relative to its value at the beginning of the makefile.
PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# warning is useful to print out values
#$(warning PROJECT_ROOT = $(PROJECT_ROOT))

# shared objects are listed here. 
# As source files are scattered across multiple folders, we need the  
OBJS = 	example/bnDataset.o \
		example/example1.o \
		example/example2.o \
		example/unitTests.o \
		example/example_istreamFromMem.o \
		example/example_csvTable.o
	
# resulting .d files (for cleaning)
DEPS:= $(patsubst %.o,%.d,$(OBJS))

MY_BUILDFLAGS = -std=c++17 -Wall -Wfatal-errors -Wextra -Wpedantic  -Weffc++
#-Wconversion 
#-Wsign-conversion -Weffc++ -Wshadow
# Eclipse passes debug/release build via this variable
ifeq ($(BUILD_MODE),debug)
	CFLAGS += -O1 -g ${MY_BUILDFLAGS}
else ifeq ($(BUILD_MODE),run)
	CFLAGS += -O3 -Wall -DNDEBUG ${MY_BUILDFLAGS}
else
	CFLAGS += -O1 -g ${MY_BUILDFLAGS}
endif
LDFLAGS += -static

all:	example1.exe example2.exe unitTests.exe example_istreamFromMem.exe example_csvTable.exe

# include generated rules that state dependencies (list of #include files per object)
-include $(DEPS)

# Flags for dependency extraction into *.d file for every object target
DEPEXTR_FLAGS = -MMD -MF $(@:.o=.d)

# rules to build toplevel executables. 
# Note: All object files must be added to DEPS (otherwise failure to recompile on header change!) 
example1.exe:	example/example1.o example/bnDataset.o
	$(CXX) $(LDFLAGS) -o $@ $^
example2.exe:	example/example2.o
	$(CXX) $(LDFLAGS) -o $@ $^
example_istreamFromMem.exe:	example/example_istreamFromMem.o
	$(CXX) $(LDFLAGS) -o $@ $^
example_csvTable.exe:	example/example_csvTable.o
	$(CXX) $(LDFLAGS) -o $@ $^
unitTests.exe:	example/unitTests.o  example/bnDataset.o
	$(CXX) $(LDFLAGS) -o $@ $^

#	# rule to build object files in OBJ (if multiple folders, use several rules)
#sampleSrcForSharedObject/%.o:	$(PROJECT_ROOT)/sampleSrcForSharedObject/%.cpp
#	# create the directory in the build folder 
#	@mkdir -p $(@D)        
#	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(DEPEXTR_FLAGS)

# rule to build object files for top level executables 
example/%.o:	$(PROJECT_ROOT)/example/%.cpp
	@mkdir -p $(@D)        
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(DEPEXTR_FLAGS)

clean:
# Note: if running the Makefile from the command line, the build folder is the top level folder.
# In this case, object files are generated in the respective source folders => careful with deleting anything.
	rm -rf ${OBJS} ${DEPS} example/*.o example/*.d *.exe