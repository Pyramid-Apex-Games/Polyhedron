# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/micha/dev/cube/Polyhedron/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug

# Include any dependencies generated for this target.
include _deps/libpng-build/CMakeFiles/pngvalid.dir/depend.make

# Include the progress variables for this target.
include _deps/libpng-build/CMakeFiles/pngvalid.dir/progress.make

# Include the compile flags for this target's objects.
include _deps/libpng-build/CMakeFiles/pngvalid.dir/flags.make

_deps/libpng-build/CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.o: _deps/libpng-build/CMakeFiles/pngvalid.dir/flags.make
_deps/libpng-build/CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.o: /Users/micha/dev/cube/Polyhedron/cmake/thirdparty/libpng/contrib/libtests/pngvalid.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object _deps/libpng-build/CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.o"
	cd /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/libpng-build && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.o   -c /Users/micha/dev/cube/Polyhedron/cmake/thirdparty/libpng/contrib/libtests/pngvalid.c

_deps/libpng-build/CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.i"
	cd /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/libpng-build && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/micha/dev/cube/Polyhedron/cmake/thirdparty/libpng/contrib/libtests/pngvalid.c > CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.i

_deps/libpng-build/CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.s"
	cd /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/libpng-build && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/micha/dev/cube/Polyhedron/cmake/thirdparty/libpng/contrib/libtests/pngvalid.c -o CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.s

# Object files for target pngvalid
pngvalid_OBJECTS = \
"CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.o"

# External object files for target pngvalid
pngvalid_EXTERNAL_OBJECTS =

_deps/libpng-build/pngvalid: _deps/libpng-build/CMakeFiles/pngvalid.dir/contrib/libtests/pngvalid.c.o
_deps/libpng-build/pngvalid: _deps/libpng-build/CMakeFiles/pngvalid.dir/build.make
_deps/libpng-build/pngvalid: _deps/libpng-build/libpng16d.16.37.0.dylib
_deps/libpng-build/pngvalid: /usr/lib/libz.dylib
_deps/libpng-build/pngvalid: _deps/libpng-build/CMakeFiles/pngvalid.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable pngvalid"
	cd /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/libpng-build && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pngvalid.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
_deps/libpng-build/CMakeFiles/pngvalid.dir/build: _deps/libpng-build/pngvalid

.PHONY : _deps/libpng-build/CMakeFiles/pngvalid.dir/build

_deps/libpng-build/CMakeFiles/pngvalid.dir/clean:
	cd /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/libpng-build && $(CMAKE_COMMAND) -P CMakeFiles/pngvalid.dir/cmake_clean.cmake
.PHONY : _deps/libpng-build/CMakeFiles/pngvalid.dir/clean

_deps/libpng-build/CMakeFiles/pngvalid.dir/depend:
	cd /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/micha/dev/cube/Polyhedron/src /Users/micha/dev/cube/Polyhedron/cmake/thirdparty/libpng /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/libpng-build /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/libpng-build/CMakeFiles/pngvalid.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : _deps/libpng-build/CMakeFiles/pngvalid.dir/depend

