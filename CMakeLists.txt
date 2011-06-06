#
# Data serialization benchmarks project.
#
cmake_minimum_required(VERSION 2.4 FATAL_ERROR)

project(bench_encoders C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(UsePkgConfig)
include(CheckCSourceCompiles)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckVariableExists)
include(CheckTypeSize)
include(CheckLibraryExists)
include(CMakeDetermineCCompiler)
include(FindThreads)
include(FindPkgConfig)
include(CustomMacros)

option(BENCH_PROTOBUF "Benchmark google protocol buffers" ON)
option(BENCH_AVRO "Benchmark Avro" ON)

option(USE_CLOCK_GETTIME "Use clock_gettime() if available, for timings." ON)
option(USE_GTIMER "Use GTimer from glib, for timings." ON)
option(USE_GETTIMEOFDAY "Use gettimeofday(), for timings." ON)

#
# Find system libraries
#
set(COMMON_CFLAGS)
set(COMMON_LDFLAGS)
set(COMMON_LIBS)

##
## Find a clock.
##
unset(HAVE_CLOCK_GETTIME CACHE)
unset(HAVE_GTIMER CACHE)
unset(HAVE_GETTIMEOFDAY CACHE)
set(FOUND_CLOCK FALSE)
if(USE_CLOCK_GETTIME)
	set(CMAKE_REQUIRED_FLAGS "-lrt")
	# check for clock_gettime() function.
	check_c_source_compiles("
		#include <time.h>
		int main() {
			struct timespec tp;
			return clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
		}" HAVE_CLOCK_GETTIME)
	set(FOUND_CLOCK ${HAVE_CLOCK_GETTIME})
	if(HAVE_CLOCK_GETTIME)
		set(COMMON_LDFLAGS "${COMMON_LDFLAGS} -lrt")
	endif(HAVE_CLOCK_GETTIME)
	set(CMAKE_REQUIRED_FLAGS "")
endif(USE_CLOCK_GETTIME)

if(USE_GTIMER AND NOT FOUND_CLOCK)
	## GLib 2.0
	pkg_search_module(GLIB glib-2.0>=2.14)
	set(COMMON_CFLAGS "${COMMON_CFLAGS} ${GLIB_CFLAGS}")
	set(COMMON_LDFLAGS "${COMMON_LDFLAGS} ${GLIB_LDFLAGS}")
	set(HAVE_GTIMER ${GLIB_FOUND})
	set(FOUND_CLOCK ${HAVE_GTIMER})
endif(USE_GTIMER AND NOT FOUND_CLOCK)

if(USE_GETTIMEOFDAY AND NOT FOUND_CLOCK)
	# check for gettimeofday() function.
	check_c_source_compiles("
		#include <sys/time.h>
		int main() {
			struct timeval tv;
			return gettimeofday(&tv, 0);
		}" HAVE_GETTIMEOFDAY)
	set(FOUND_CLOCK ${HAVE_GETTIMEOFDAY})
endif(USE_GETTIMEOFDAY AND NOT FOUND_CLOCK)

## Protobuf
if(BENCH_PROTOBUF)
	pkg_search_module(PROTOBUF REQUIRED protobuf>=2.0)
	set(COMMON_CFLAGS "${COMMON_CFLAGS} ${PROTOBUF_CFLAGS}")
	set(COMMON_LDFLAGS "${COMMON_LDFLAGS} ${PROTOBUF_LDFLAGS}")
endif(BENCH_PROTOBUF)

## Avro
if(BENCH_AVRO)
	find_package(LibAvro)
	include_directories(${LIBAVRO_INCLUDE_DIR})
	set(COMMON_LIBS "${COMMON_LIBS};${LIBAVRO_LIBRARY}")
	set(CMAKE_REQUIRED_LIBRARIES
					"${CMAKE_REQUIRED_LIBRARIES};${LIBAVRO_LIBRARY}")
	# check for avro_array_get() function.
	check_c_source_compiles("
		#include <avro.h>
		int main() {
			avro_datum_t test, array;
			return avro_array_get(array, 0, &test);
		}" HAVE_avro_array_get)
	set(CMAKE_REQUIRED_LIBRARIES "")
endif(BENCH_AVRO)

## generate config.h
configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
        ${CMAKE_SOURCE_DIR}/build/config.h)

set(COMMON_CFLAGS "${COMMON_CFLAGS} -include ${CMAKE_SOURCE_DIR}/build/config.h")

#
# Setup CMAKE_C_FLAGS* and CMAKE_CXX_FLAGS*
#
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -Wall -Wextra -Wshadow -W -pedantic -std=gnu99 -fgnu89-inline")
	set(CMAKE_C_FLAGS_RELEASE        "${CMAKE_C_FLAGS_RELEASE}     -O3 -march=athlon64 -g")
	set(CMAKE_C_FLAGS_DEBUG          "${CMAKE_C_FLAGS_DEBUG}       -O0 -g")
	set(CMAKE_C_FLAGS_PROFILE        "${CMAKE_C_FLAGS_PROFILE}     -O2 -g -DNDEBUG")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_WITHDEBINFO} -O2 -g")
endif(CMAKE_COMPILER_IS_GNUCC)
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -Wall -Wextra -Wshadow -W -pedantic -std=gnu++0x -Wno-shadow")
	set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}     -O3 -march=athlon64 -g")
	set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}       -O0 -g")
	set(CMAKE_CXX_FLAGS_PROFILE        "${CMAKE_CXX_FLAGS_PROFILE}     -O2 -g -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_WITHDEBINFO} -O2 -g")
endif(CMAKE_COMPILER_IS_GNUCXX)

#
# Subdirectories
#
add_subdirectory(src build)


