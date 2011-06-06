#
# encoder/decoder benchmark.
#

set(AVRO_SRC
	bench_avro.c
	media.avro
)
set(PROTOBUF_SRC
	bench_protobuf.cc
	media.proto
)
set(BENCH_SRC
	bench_enc.c
)

if(BENCH_AVRO)
	set(BENCH_SRC ${AVRO_SRC} ${BENCH_SRC})
endif(BENCH_AVRO)
if(BENCH_PROTOBUF)
	set(BENCH_SRC ${PROTOBUF_SRC} ${BENCH_SRC})
endif(BENCH_PROTOBUF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

#
# Protocol Buffer processor
#
protobuf_compile(BENCH_SRC)

#
# Avro processor
#
avro_compile(BENCH_SRC)

#
# Project executables
#
add_executable(bench_enc ${BENCH_SRC})
add_target_properties(bench_enc COMPILE_FLAGS "${COMMON_CFLAGS}")
add_target_properties(bench_enc LINK_FLAGS "${LD_FLAGS} ${COMMON_LDFLAGS}")
target_link_libraries(bench_enc ${COMMON_LIBS})

