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
include test/CMakeFiles/circulation_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/circulation_test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/circulation_test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/circulation_test.dir/flags.make

test/CMakeFiles/circulation_test.dir/circulation_test.cc.o: test/CMakeFiles/circulation_test.dir/flags.make
test/CMakeFiles/circulation_test.dir/circulation_test.cc.o: test/circulation_test.cc
test/CMakeFiles/circulation_test.dir/circulation_test.cc.o: test/CMakeFiles/circulation_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/circulation_test.dir/circulation_test.cc.o"
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/circulation_test.dir/circulation_test.cc.o -MF CMakeFiles/circulation_test.dir/circulation_test.cc.o.d -o CMakeFiles/circulation_test.dir/circulation_test.cc.o -c /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test/circulation_test.cc

test/CMakeFiles/circulation_test.dir/circulation_test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/circulation_test.dir/circulation_test.cc.i"
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test/circulation_test.cc > CMakeFiles/circulation_test.dir/circulation_test.cc.i

test/CMakeFiles/circulation_test.dir/circulation_test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/circulation_test.dir/circulation_test.cc.s"
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test/circulation_test.cc -o CMakeFiles/circulation_test.dir/circulation_test.cc.s

# Object files for target circulation_test
circulation_test_OBJECTS = \
"CMakeFiles/circulation_test.dir/circulation_test.cc.o"

# External object files for target circulation_test
circulation_test_EXTERNAL_OBJECTS =

test/circulation_test: test/CMakeFiles/circulation_test.dir/circulation_test.cc.o
test/circulation_test: test/CMakeFiles/circulation_test.dir/build.make
test/circulation_test: lemon/libemon.a
test/circulation_test: /lib/libglpk.so
test/circulation_test: test/CMakeFiles/circulation_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable circulation_test"
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/circulation_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/circulation_test.dir/build: test/circulation_test
.PHONY : test/CMakeFiles/circulation_test.dir/build

test/CMakeFiles/circulation_test.dir/clean:
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test && $(CMAKE_COMMAND) -P CMakeFiles/circulation_test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/circulation_test.dir/clean

test/CMakeFiles/circulation_test.dir/depend:
	cd /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1 /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1 /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test /home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/test/CMakeFiles/circulation_test.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : test/CMakeFiles/circulation_test.dir/depend

