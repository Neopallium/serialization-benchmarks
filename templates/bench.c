/*****
vi commands:
:%s/<benchname>//g
******/

#include <stdio.h>

#include "bench_enc.h"

static void * create_<benchname>() {
	return NULL;
}

static void * build_<benchname>(void *obj) {
	return obj;
}

static void free_<benchname>(void *obj) {
}

static void check_media_<benchname>(void *obj) {
}

static void check_all_<benchname>(void *obj) {

	check_media_<benchname>(obj);
}

static void *init_<benchname>(char *buf, size_t len) {
	return NULL;
}

static void cleanup_<benchname>(void *state) {
}

static size_t encode_<benchname>(void *state, void *obj, char *buf, size_t len) {
	return 0;
}

static void * decode_<benchname>(void *state, void *obj, char *buf, size_t len) {
	return obj;
}

BENCH_ENC_REG(<benchname>, msg_name, "Benchmark: <benchname> system.")

