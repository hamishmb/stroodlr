# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/hamish/Programming/Projects/Stroodlr

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hamish/Programming/Projects/Stroodlr/build

# Include any dependencies generated for this target.
include CMakeFiles/stroodlrd.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/stroodlrd.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/stroodlrd.dir/flags.make

CMakeFiles/stroodlrd.dir/src/server.cpp.o: CMakeFiles/stroodlrd.dir/flags.make
CMakeFiles/stroodlrd.dir/src/server.cpp.o: ../src/server.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hamish/Programming/Projects/Stroodlr/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/stroodlrd.dir/src/server.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/stroodlrd.dir/src/server.cpp.o -c /home/hamish/Programming/Projects/Stroodlr/src/server.cpp

CMakeFiles/stroodlrd.dir/src/server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/stroodlrd.dir/src/server.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hamish/Programming/Projects/Stroodlr/src/server.cpp > CMakeFiles/stroodlrd.dir/src/server.cpp.i

CMakeFiles/stroodlrd.dir/src/server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/stroodlrd.dir/src/server.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hamish/Programming/Projects/Stroodlr/src/server.cpp -o CMakeFiles/stroodlrd.dir/src/server.cpp.s

CMakeFiles/stroodlrd.dir/src/server.cpp.o.requires:

.PHONY : CMakeFiles/stroodlrd.dir/src/server.cpp.o.requires

CMakeFiles/stroodlrd.dir/src/server.cpp.o.provides: CMakeFiles/stroodlrd.dir/src/server.cpp.o.requires
	$(MAKE) -f CMakeFiles/stroodlrd.dir/build.make CMakeFiles/stroodlrd.dir/src/server.cpp.o.provides.build
.PHONY : CMakeFiles/stroodlrd.dir/src/server.cpp.o.provides

CMakeFiles/stroodlrd.dir/src/server.cpp.o.provides.build: CMakeFiles/stroodlrd.dir/src/server.cpp.o


CMakeFiles/stroodlrd.dir/include/tools.cpp.o: CMakeFiles/stroodlrd.dir/flags.make
CMakeFiles/stroodlrd.dir/include/tools.cpp.o: ../include/tools.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hamish/Programming/Projects/Stroodlr/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/stroodlrd.dir/include/tools.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/stroodlrd.dir/include/tools.cpp.o -c /home/hamish/Programming/Projects/Stroodlr/include/tools.cpp

CMakeFiles/stroodlrd.dir/include/tools.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/stroodlrd.dir/include/tools.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hamish/Programming/Projects/Stroodlr/include/tools.cpp > CMakeFiles/stroodlrd.dir/include/tools.cpp.i

CMakeFiles/stroodlrd.dir/include/tools.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/stroodlrd.dir/include/tools.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hamish/Programming/Projects/Stroodlr/include/tools.cpp -o CMakeFiles/stroodlrd.dir/include/tools.cpp.s

CMakeFiles/stroodlrd.dir/include/tools.cpp.o.requires:

.PHONY : CMakeFiles/stroodlrd.dir/include/tools.cpp.o.requires

CMakeFiles/stroodlrd.dir/include/tools.cpp.o.provides: CMakeFiles/stroodlrd.dir/include/tools.cpp.o.requires
	$(MAKE) -f CMakeFiles/stroodlrd.dir/build.make CMakeFiles/stroodlrd.dir/include/tools.cpp.o.provides.build
.PHONY : CMakeFiles/stroodlrd.dir/include/tools.cpp.o.provides

CMakeFiles/stroodlrd.dir/include/tools.cpp.o.provides.build: CMakeFiles/stroodlrd.dir/include/tools.cpp.o


CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o: CMakeFiles/stroodlrd.dir/flags.make
CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o: ../include/loggertools.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hamish/Programming/Projects/Stroodlr/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o -c /home/hamish/Programming/Projects/Stroodlr/include/loggertools.cpp

CMakeFiles/stroodlrd.dir/include/loggertools.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/stroodlrd.dir/include/loggertools.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hamish/Programming/Projects/Stroodlr/include/loggertools.cpp > CMakeFiles/stroodlrd.dir/include/loggertools.cpp.i

CMakeFiles/stroodlrd.dir/include/loggertools.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/stroodlrd.dir/include/loggertools.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hamish/Programming/Projects/Stroodlr/include/loggertools.cpp -o CMakeFiles/stroodlrd.dir/include/loggertools.cpp.s

CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o.requires:

.PHONY : CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o.requires

CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o.provides: CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o.requires
	$(MAKE) -f CMakeFiles/stroodlrd.dir/build.make CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o.provides.build
.PHONY : CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o.provides

CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o.provides.build: CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o


CMakeFiles/stroodlrd.dir/include/servertools.cpp.o: CMakeFiles/stroodlrd.dir/flags.make
CMakeFiles/stroodlrd.dir/include/servertools.cpp.o: ../include/servertools.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hamish/Programming/Projects/Stroodlr/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/stroodlrd.dir/include/servertools.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/stroodlrd.dir/include/servertools.cpp.o -c /home/hamish/Programming/Projects/Stroodlr/include/servertools.cpp

CMakeFiles/stroodlrd.dir/include/servertools.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/stroodlrd.dir/include/servertools.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hamish/Programming/Projects/Stroodlr/include/servertools.cpp > CMakeFiles/stroodlrd.dir/include/servertools.cpp.i

CMakeFiles/stroodlrd.dir/include/servertools.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/stroodlrd.dir/include/servertools.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hamish/Programming/Projects/Stroodlr/include/servertools.cpp -o CMakeFiles/stroodlrd.dir/include/servertools.cpp.s

CMakeFiles/stroodlrd.dir/include/servertools.cpp.o.requires:

.PHONY : CMakeFiles/stroodlrd.dir/include/servertools.cpp.o.requires

CMakeFiles/stroodlrd.dir/include/servertools.cpp.o.provides: CMakeFiles/stroodlrd.dir/include/servertools.cpp.o.requires
	$(MAKE) -f CMakeFiles/stroodlrd.dir/build.make CMakeFiles/stroodlrd.dir/include/servertools.cpp.o.provides.build
.PHONY : CMakeFiles/stroodlrd.dir/include/servertools.cpp.o.provides

CMakeFiles/stroodlrd.dir/include/servertools.cpp.o.provides.build: CMakeFiles/stroodlrd.dir/include/servertools.cpp.o


# Object files for target stroodlrd
stroodlrd_OBJECTS = \
"CMakeFiles/stroodlrd.dir/src/server.cpp.o" \
"CMakeFiles/stroodlrd.dir/include/tools.cpp.o" \
"CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o" \
"CMakeFiles/stroodlrd.dir/include/servertools.cpp.o"

# External object files for target stroodlrd
stroodlrd_EXTERNAL_OBJECTS =

../dist/stroodlrd: CMakeFiles/stroodlrd.dir/src/server.cpp.o
../dist/stroodlrd: CMakeFiles/stroodlrd.dir/include/tools.cpp.o
../dist/stroodlrd: CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o
../dist/stroodlrd: CMakeFiles/stroodlrd.dir/include/servertools.cpp.o
../dist/stroodlrd: CMakeFiles/stroodlrd.dir/build.make
../dist/stroodlrd: /usr/lib/x86_64-linux-gnu/libboost_system.so
../dist/stroodlrd: CMakeFiles/stroodlrd.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hamish/Programming/Projects/Stroodlr/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable ../dist/stroodlrd"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/stroodlrd.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/stroodlrd.dir/build: ../dist/stroodlrd

.PHONY : CMakeFiles/stroodlrd.dir/build

CMakeFiles/stroodlrd.dir/requires: CMakeFiles/stroodlrd.dir/src/server.cpp.o.requires
CMakeFiles/stroodlrd.dir/requires: CMakeFiles/stroodlrd.dir/include/tools.cpp.o.requires
CMakeFiles/stroodlrd.dir/requires: CMakeFiles/stroodlrd.dir/include/loggertools.cpp.o.requires
CMakeFiles/stroodlrd.dir/requires: CMakeFiles/stroodlrd.dir/include/servertools.cpp.o.requires

.PHONY : CMakeFiles/stroodlrd.dir/requires

CMakeFiles/stroodlrd.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/stroodlrd.dir/cmake_clean.cmake
.PHONY : CMakeFiles/stroodlrd.dir/clean

CMakeFiles/stroodlrd.dir/depend:
	cd /home/hamish/Programming/Projects/Stroodlr/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hamish/Programming/Projects/Stroodlr /home/hamish/Programming/Projects/Stroodlr /home/hamish/Programming/Projects/Stroodlr/build /home/hamish/Programming/Projects/Stroodlr/build /home/hamish/Programming/Projects/Stroodlr/build/CMakeFiles/stroodlrd.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/stroodlrd.dir/depend

