# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

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
CMAKE_COMMAND = /snap/cmake/1384/bin/cmake

# The command to remove a file.
RM = /snap/cmake/1384/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build

# Include any dependencies generated for this target.
include lib/libdyn/CMakeFiles/DynamiCLib.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include lib/libdyn/CMakeFiles/DynamiCLib.dir/compiler_depend.make

# Include the progress variables for this target.
include lib/libdyn/CMakeFiles/DynamiCLib.dir/progress.make

# Include the compile flags for this target's objects.
include lib/libdyn/CMakeFiles/DynamiCLib.dir/flags.make

lib/libdyn/CMakeFiles/DynamiCLib.dir/src/emergency.c.o: lib/libdyn/CMakeFiles/DynamiCLib.dir/flags.make
lib/libdyn/CMakeFiles/DynamiCLib.dir/src/emergency.c.o: /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/lib/libdyn/src/emergency.c
lib/libdyn/CMakeFiles/DynamiCLib.dir/src/emergency.c.o: lib/libdyn/CMakeFiles/DynamiCLib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object lib/libdyn/CMakeFiles/DynamiCLib.dir/src/emergency.c.o"
	cd /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/lib/libdyn && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT lib/libdyn/CMakeFiles/DynamiCLib.dir/src/emergency.c.o -MF CMakeFiles/DynamiCLib.dir/src/emergency.c.o.d -o CMakeFiles/DynamiCLib.dir/src/emergency.c.o -c /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/lib/libdyn/src/emergency.c

lib/libdyn/CMakeFiles/DynamiCLib.dir/src/emergency.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/DynamiCLib.dir/src/emergency.c.i"
	cd /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/lib/libdyn && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/lib/libdyn/src/emergency.c > CMakeFiles/DynamiCLib.dir/src/emergency.c.i

lib/libdyn/CMakeFiles/DynamiCLib.dir/src/emergency.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/DynamiCLib.dir/src/emergency.c.s"
	cd /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/lib/libdyn && /bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/lib/libdyn/src/emergency.c -o CMakeFiles/DynamiCLib.dir/src/emergency.c.s

# Object files for target DynamiCLib
DynamiCLib_OBJECTS = \
"CMakeFiles/DynamiCLib.dir/src/emergency.c.o"

# External object files for target DynamiCLib
DynamiCLib_EXTERNAL_OBJECTS =

lib/libdyn/libDynamiCLib.so: lib/libdyn/CMakeFiles/DynamiCLib.dir/src/emergency.c.o
lib/libdyn/libDynamiCLib.so: lib/libdyn/CMakeFiles/DynamiCLib.dir/build.make
lib/libdyn/libDynamiCLib.so: lib/libdyn/CMakeFiles/DynamiCLib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared library libDynamiCLib.so"
	cd /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/lib/libdyn && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/DynamiCLib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/libdyn/CMakeFiles/DynamiCLib.dir/build: lib/libdyn/libDynamiCLib.so
.PHONY : lib/libdyn/CMakeFiles/DynamiCLib.dir/build

lib/libdyn/CMakeFiles/DynamiCLib.dir/clean:
	cd /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/lib/libdyn && $(CMAKE_COMMAND) -P CMakeFiles/DynamiCLib.dir/cmake_clean.cmake
.PHONY : lib/libdyn/CMakeFiles/DynamiCLib.dir/clean

lib/libdyn/CMakeFiles/DynamiCLib.dir/depend:
	cd /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/lib/libdyn /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/lib/libdyn /home/david/ingenieria_en_computacion/quinto_anio/sistemas_operativos2/practico/tps/lab1/2024/lab1_-daviddaguero/build/lib/libdyn/CMakeFiles/DynamiCLib.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : lib/libdyn/CMakeFiles/DynamiCLib.dir/depend
