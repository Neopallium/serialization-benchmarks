/***************************************************************************
 * Copyright (C) 2007-2010 by Robert G. Jakabosky <bobby@neoawareness.com> *
 *                                                                         *
 ***************************************************************************/
#ifndef __BENCH_ENC_H__
#define __BENCH_ENC_H__

#include <stdlib.h>
#include <string.h>

#define STATIC_CONSTRUCTOR(name) static void __attribute__ ((constructor)) init_ ## name (void)

#define BENCH_ENC_REG(bench_name, msg_name, _desc) \
	static BenchEncInfo bench_enc_info_ ## bench_name = { \
		#msg_name, \
		create_ ## bench_name, \
		build_ ## bench_name, \
		free_ ## bench_name, \
		enc_size_ ## bench_name, \
		encode_ ## bench_name, \
		decode_ ## bench_name, \
		check_all_ ## bench_name, \
		check_part_ ## bench_name, \
		init_ ## bench_name, \
		cleanup_ ## bench_name, \
		#bench_name, \
		_desc, \
		NULL, \
		NULL, \
		NULL, \
		0, \
		NULL, \
		NULL \
	}; \
	STATIC_CONSTRUCTOR(bench_ ## bench_name) { \
		bench_enc_reg(&(bench_enc_info_ ## bench_name)); \
	}

#define check_val(expr) \
	if(expr) {} else { \
		printf("Field check failed: " #expr "\n"); \
		exit(EXIT_FAILURE); \
	}

typedef struct BenchEncInfo BenchEncInfo;

typedef void *(*create_func)();
typedef void *(*build_func)(void *obj);
typedef void (*free_func)(void *obj);
typedef void (*check_func)(void *obj);

typedef void *(*init_func)(char *buf, size_t len);
typedef void (*cleanup_func)(void *state);

typedef size_t (*enc_size_func)(void *state, void *obj);
typedef size_t (*encode_func)(void *state, void *obj, char *buf, size_t len);
typedef void *(*decode_func)(void *state, void *obj, char *buf, size_t len);

struct BenchEncInfo {
	const char    *msg_name;
	create_func   create;
	build_func    build;
	free_func     free;
	enc_size_func enc_size;
	encode_func   encode;
	decode_func   decode;
	check_func    check_all;
	check_func    check_partial;
	init_func     init;
	cleanup_func  cleanup;
	const char    *name;
	const char    *desc;
	double        *stats;
	uint32_t      *counts;
	size_t        *bytes;
	uint32_t      encode_size;
	BenchEncInfo	*msg_next;
	BenchEncInfo	*next;
};

extern void bench_enc_reg(BenchEncInfo *bench);

extern BenchEncInfo *bench_enc_get_next_type(const char *type, BenchEncInfo *bench);

#endif /* __BENCH_ENC_H__ */
