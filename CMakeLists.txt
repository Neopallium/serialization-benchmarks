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

#
# Find system libraries
#
set(COMMON_CFLAGS)
set(COMMON_LDFLAGS)
set(COMMON_LIBS)

## GLib 2.0
pkg_search_module(GLIB REQUIRED glib-2.0>=2.14)
set(COMMON_CFLAGS "${COMMON_CFLAGS} ${GLIB_CFLAGS}")
set(COMMON_LDFLAGS "${COMMON_LDFLAGS} ${GLIB_LDFLAGS}")

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
endif(BENCH_AVRO)

## generate config.h
configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake
        ${CMAKE_SOURCE_DIR}/build/config.h)

set(COMMON_CFLAGS "${COMMON_CFLAGS} -include config.h")

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


