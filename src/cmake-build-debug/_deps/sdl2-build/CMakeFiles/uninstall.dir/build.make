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

# Utility rule file for uninstall.

# Include the progress variables for this target.
include _deps/sdl2-build/CMakeFiles/uninstall.dir/progress.make

_deps/sdl2-build/CMakeFiles/uninstall:
	cd /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/sdl2-build && /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -P /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/sdl2-build/cmake_uninstall.cmake

uninstall: _deps/sdl2-build/CMakeFiles/uninstall
uninstall: _deps/sdl2-build/CMakeFiles/uninstall.dir/build.make

.PHONY : uninstall

# Rule to build all files generated by this target.
_deps/sdl2-build/CMakeFiles/uninstall.dir/build: uninstall

.PHONY : _deps/sdl2-build/CMakeFiles/uninstall.dir/build

_deps/sdl2-build/CMakeFiles/uninstall.dir/clean:
	cd /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/sdl2-build && $(CMAKE_COMMAND) -P CMakeFiles/uninstall.dir/cmake_clean.cmake
.PHONY : _deps/sdl2-build/CMakeFiles/uninstall.dir/clean

_deps/sdl2-build/CMakeFiles/uninstall.dir/depend:
	cd /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/micha/dev/cube/Polyhedron/src /Users/micha/dev/cube/Polyhedron/cmake/thirdparty/SDL2-2.0.12 /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/sdl2-build /Users/micha/dev/cube/Polyhedron/src/cmake-build-debug/_deps/sdl2-build/CMakeFiles/uninstall.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : _deps/sdl2-build/CMakeFiles/uninstall.dir/depend

