# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1

# Include any dependencies generated for this target.
include test/CMakeFiles/lgf_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/lgf_test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/lgf_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/lgf_test.dir/flags.make

test/CMakeFiles/lgf_test.dir/lgf_test.cc.o: test/CMakeFiles/lgf_test.dir/flags.make
test/CMakeFiles/lgf_test.dir/lgf_test.cc.o: test/lgf_test.cc
test/CMakeFiles/lgf_test.dir/lgf_test.cc.o: test/CMakeFiles/lgf_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/lgf_test.dir/lgf_test.cc.o"
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/lgf_test.dir/lgf_test.cc.o -MF CMakeFiles/lgf_test.dir/lgf_test.cc.o.d -o CMakeFiles/lgf_test.dir/lgf_test.cc.o -c /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test/lgf_test.cc

test/CMakeFiles/lgf_test.dir/lgf_test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/lgf_test.dir/lgf_test.cc.i"
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test/lgf_test.cc > CMakeFiles/lgf_test.dir/lgf_test.cc.i

test/CMakeFiles/lgf_test.dir/lgf_test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/lgf_test.dir/lgf_test.cc.s"
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test/lgf_test.cc -o CMakeFiles/lgf_test.dir/lgf_test.cc.s

# Object files for target lgf_test
lgf_test_OBJECTS = \
"CMakeFiles/lgf_test.dir/lgf_test.cc.o"

# External object files for target lgf_test
lgf_test_EXTERNAL_OBJECTS =

test/lgf_test: test/CMakeFiles/lgf_test.dir/lgf_test.cc.o
test/lgf_test: test/CMakeFiles/lgf_test.dir/build.make
test/lgf_test: lemon/libemon.a
test/lgf_test: /lib/libglpk.so
test/lgf_test: test/CMakeFiles/lgf_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable lgf_test"
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lgf_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/lgf_test.dir/build: test/lgf_test
.PHONY : test/CMakeFiles/lgf_test.dir/build

test/CMakeFiles/lgf_test.dir/clean:
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test && $(CMAKE_COMMAND) -P CMakeFiles/lgf_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/lgf_test.dir/clean

test/CMakeFiles/lgf_test.dir/depend:
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1 /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1 /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test/CMakeFiles/lgf_test.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : test/CMakeFiles/lgf_test.dir/depend

