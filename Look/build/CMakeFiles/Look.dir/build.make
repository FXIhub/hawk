# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.4

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
.SUFFIXES:

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /sw/bin/cmake

# The command to remove a file.
RM = /sw/bin/cmake -E remove -f

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /sw/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/ekeberg/programs/Qt/Look

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/ekeberg/programs/Qt/Look/build

# Include any dependencies generated for this target.
include CMakeFiles/Look.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Look.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Look.dir/flags.make

CMakeFiles/Look.dir/depend.make.mark: CMakeFiles/Look.dir/flags.make
CMakeFiles/Look.dir/depend.make.mark: ../main.cpp

CMakeFiles/Look.dir/main.o: CMakeFiles/Look.dir/flags.make
CMakeFiles/Look.dir/main.o: ../main.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/ekeberg/programs/Qt/Look/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/Look.dir/main.o"
	/usr/bin/c++   $(CXX_FLAGS) -o CMakeFiles/Look.dir/main.o -c /Users/ekeberg/programs/Qt/Look/main.cpp

CMakeFiles/Look.dir/main.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Look.dir/main.i"
	/usr/bin/c++  $(CXX_FLAGS) -E /Users/ekeberg/programs/Qt/Look/main.cpp > CMakeFiles/Look.dir/main.i

CMakeFiles/Look.dir/main.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Look.dir/main.s"
	/usr/bin/c++  $(CXX_FLAGS) -S /Users/ekeberg/programs/Qt/Look/main.cpp -o CMakeFiles/Look.dir/main.s

CMakeFiles/Look.dir/main.o.requires:

CMakeFiles/Look.dir/main.o.provides: CMakeFiles/Look.dir/main.o.requires
	$(MAKE) -f CMakeFiles/Look.dir/build.make CMakeFiles/Look.dir/main.o.provides.build

CMakeFiles/Look.dir/main.o.provides.build: CMakeFiles/Look.dir/main.o

CMakeFiles/Look.dir/depend: CMakeFiles/Look.dir/depend.make.mark

CMakeFiles/Look.dir/depend.make.mark:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --magenta --bold "Scanning dependencies of target Look"
	cd /Users/ekeberg/programs/Qt/Look/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/ekeberg/programs/Qt/Look /Users/ekeberg/programs/Qt/Look /Users/ekeberg/programs/Qt/Look/build /Users/ekeberg/programs/Qt/Look/build /Users/ekeberg/programs/Qt/Look/build/CMakeFiles/Look.dir/DependInfo.cmake

# Object files for target Look
Look_OBJECTS = \
"CMakeFiles/Look.dir/main.o"

# External object files for target Look
Look_EXTERNAL_OBJECTS =

Look: CMakeFiles/Look.dir/main.o
Look: CMakeFiles/Look.dir/build.make
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable Look"
	$(CMAKE_COMMAND) -P CMakeFiles/Look.dir/cmake_clean_target.cmake
	/usr/bin/c++     -headerpad_max_install_names -fPIC $(Look_OBJECTS) $(Look_EXTERNAL_OBJECTS)  -o Look  

# Rule to build all files generated by this target.
CMakeFiles/Look.dir/build: Look

CMakeFiles/Look.dir/requires: CMakeFiles/Look.dir/main.o.requires

CMakeFiles/Look.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Look.dir/cmake_clean.cmake

