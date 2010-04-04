
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/user.h>
#include "bench_enc.h"

#include "protobuf.h"

#define MAX_BUFFER_SIZE 128 * 1024

static int loop_multipler = 100;

#define LOOP_YIELD         (1 * loop_multipler)
#define LOOP_ENC_DEC       (100 * loop_multipler)
#define LOOP_CREATE_DELETE (20 * loop_multipler)
#define LOOP_ROUND_TRIP    (20 * loop_multipler)
#define LOOP_CREATE        (2000)

/* microseconds per millisecond */
#define USEC_PER_MSEC       1000ULL
/* microseconds per second */
#define USEC_PER_SEC     1000000ULL
/* nanoseconds per microsecond */
#define NSEC_PER_USEC       1000ULL
/* nanoseconds per millisecond */
#define NSEC_PER_MSEC    1000000ULL
/* nanoseconds per second */
#define NSEC_PER_SEC  1000000000ULL

#ifdef HAVE_CLOCK_GETTIME
#include <time.h>
#include <stdio.h>
#include <errno.h>
struct BTimer {
	double start;
	double end;
};
typedef struct BTimer *BTimer;

static inline double get_time_secs() {
  struct timespec tp;
  if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&tp) != 0) {
    perror("clock_gettime");
    return -1.0;
  }
  return (((double)tp.tv_nsec / NSEC_PER_SEC) + (tp.tv_sec));
}
#define btimer_new() (BTimer)calloc(1, sizeof(struct BTimer));
#define btimer_free(timer) free(timer);
#define btimer_start(timer) (timer)->start = get_time_secs()
#define btimer_stop(timer) (timer)->end = get_time_secs()
#define btimer_elapsed(timer) ((timer)->end - (timer)->start)
#else /* ifdef HAVE_CLOCK_GETTIME. */

#ifdef HAVE_GTIMER
#include <glib.h>
typedef GTimer *BTimer;
#define btimer_new() g_timer_new()
#define btimer_free(timer) g_timer_destroy(timer)
#define btimer_start(timer) g_timer_start(timer)
#define btimer_stop(timer) g_timer_stop(timer)
#define btimer_elapsed(timer) g_timer_elapsed(timer, NULL)
#else /* ifdef HAVE_GTIMER. */

#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
struct BTimer {
	double start;
	double end;
};
typedef struct BTimer *BTimer;

static inline double get_time_secs() {
  struct timeval tv;
  if(gettimeofday(&tv, NULL) != 0) {
    perror("gettimeofday");
    return -1.0;
  }
  return (((double)tv.tv_usec / USEC_PER_SEC) + (tv.tv_sec));
}
#define btimer_new() (BTimer)calloc(1, sizeof(struct BTimer));
#define btimer_free(timer) free(timer);
#define btimer_start(timer) (timer)->start = get_time_secs()
#define btimer_stop(timer) (timer)->end = get_time_secs()
#define btimer_elapsed(timer) ((timer)->end - (timer)->start)
#else /* ifdef HAVE_GETTIMEOFDAY. */

typedef void *BTimer;
#define btimer_new() NULL
#define btimer_free(timer)
#define btimer_start(timer)
#define btimer_stop(timer)
#define btimer_elapsed(timer) 1
#endif
#endif
#endif

typedef struct BenchEnc {
	BTimer       timer;
	char         *buffer;
	size_t       len;
	BenchEncInfo *info;
	void         *state;
} BenchEnc;

static BenchEnc *bench_enc_new(BenchEncInfo *info) {
	BenchEnc *bench = NULL;
	bench = (BenchEnc *)malloc(sizeof(BenchEnc));
	bench->info = info;
	/* create work buffer. */
	bench->buffer = malloc(MAX_BUFFER_SIZE);
	bench->len = MAX_BUFFER_SIZE;

	/* create timer. */
	bench->timer = btimer_new();

	/* init. bench state. */
	bench->state = info->init(bench->buffer, bench->len);

	return bench;
}

static void bench_enc_free(BenchEnc *bench) {
	if(bench->timer) {
		btimer_free(bench->timer);
		bench->timer = NULL;
	}
	/* cleanup bench state. */
	if(bench->info) {
		bench->info->cleanup(bench->state);
	}

	/* free work buffer. */
	if(bench->buffer) {
		free(bench->buffer);
		bench->buffer = NULL;
	}
	free(bench);
}

static BenchEncInfo *bench_enc_list = NULL;
void bench_enc_reg(BenchEncInfo *test) {
	test->next = bench_enc_list;
	bench_enc_list = test;
}

BenchEncInfo *bench_enc_get_next(BenchEncInfo *info) {
	if(info == NULL) {
		return bench_enc_list;
	}
	return info->next;
}

static void print_bench_stat(const char *action, const char *name, BTimer timer, uint32_t count) {
	double secs;
	secs = btimer_elapsed(timer);
	printf("%s: %s: %d at %1.0f per second\n", name, action, count,
		(double)(count / secs));
	printf("%s: %3.3f usecs\n", name, ((double)((secs * 1000000) / count)));
	printf("%s: Elapsed time: %f seconds\n\n", name, secs);
}

static double bench_create_empty_objects(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	void         *obj = NULL;
	uint32_t     n;

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->create();
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	print_bench_stat("create & delete empty object", info->name, timer, count);
	return btimer_elapsed(timer);
}

static double bench_create_full_objects(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	void         *obj = NULL;
	uint32_t     n;

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->build(NULL);
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	print_bench_stat("create & delete full object", info->name, timer, count);
	return btimer_elapsed(timer);
}

static double bench_encode_different_objects(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	uint32_t     n;

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->build(NULL);
		len = info->encode(state, obj, buffer, buflen);
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	printf("obj encode len = %zu\n", len);
	print_bench_stat("encode different objects", info->name, timer, count);
	return btimer_elapsed(timer);
}

static double bench_encode_same_object(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	uint32_t     n;

	obj = info->build(NULL);
	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		len = info->encode(state, obj, buffer, buflen);
	}
	/* bench finished */
	btimer_stop(timer);
	printf("obj encode len = %zu\n", len);
	print_bench_stat("encode same object", info->name, timer, count);
	info->free(obj);
	return btimer_elapsed(timer);
}

static double bench_decode_object(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	uint32_t     n;

	/* encode object. */
	obj = info->build(NULL);
	len = info->encode(state, obj, buffer, buflen);
	info->free(obj);
	printf("obj len = %zu\n", len);

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->decode(state, NULL, buffer, len);
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	print_bench_stat("decode object", info->name, timer, count);
	return btimer_elapsed(timer);
}

static double bench_decode_same_object(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	uint32_t     n;

	/* encode object. */
	obj = info->build(NULL);
	len = info->encode(state, obj, buffer, buflen);
	info->free(obj);
	obj = NULL;
	printf("obj len = %zu\n", len);

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->decode(state, obj, buffer, len);
	}
	info->free(obj);
	/* bench finished */
	btimer_stop(timer);
	print_bench_stat("decode same object", info->name, timer, count);
	return btimer_elapsed(timer);
}

static double bench_decode_object_check_all(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	uint32_t     n;

	/* encode object. */
	obj = info->build(NULL);
	len = info->encode(state, obj, buffer, buflen);
	info->free(obj);
	printf("obj len = %zu\n", len);

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->decode(state, NULL, buffer, len);
		info->check_all(obj);
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	print_bench_stat("decode object & check all fields", info->name, timer, count);
	return btimer_elapsed(timer);
}

static double bench_decode_object_check_media(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	uint32_t     n;

	/* encode object. */
	obj = info->build(NULL);
	len = info->encode(state, obj, buffer, buflen);
	info->free(obj);
	printf("obj len = %zu\n", len);

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->decode(state, NULL, buffer, len);
		info->check_media(obj);
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	print_bench_stat("decode object & check media field", info->name, timer, count);
	return btimer_elapsed(timer);
}

static int bench_enc_run(BenchEncInfo *info) {
	BenchEnc *bench = NULL;
	int      rc = 0;
	void *obj;

	printf("Benchmark: %s\n", info->name);
	bench = bench_enc_new(info);

	/* test check_* functions. */
	obj = info->build(NULL);
	info->check_all(obj);
	info->free(obj);

	bench_create_empty_objects(bench, LOOP_CREATE_DELETE);
	bench_create_full_objects(bench, LOOP_CREATE_DELETE);

	bench_encode_different_objects(bench, LOOP_ENC_DEC);
	bench_encode_same_object(bench, LOOP_ENC_DEC);

	bench_decode_object(bench, LOOP_ENC_DEC);
	bench_decode_same_object(bench, LOOP_ENC_DEC);
	bench_decode_object_check_all(bench, LOOP_ENC_DEC);
	bench_decode_object_check_media(bench, LOOP_ENC_DEC);

	bench_enc_free(bench);
	return rc;
}

static void print_usage(char *name) {
	printf("Usage: %s [--dump] [-r] [-s <hex seed>] <test name> [<test options>]\n", name);
}

int main(int argc, char **argv) {
	BenchEncInfo *info = NULL;
	int arg_offset = 1;
	int rc = 0;
	char c;

	init_protobuf();

	// find test name and parse common args.
	while(argc > arg_offset) {
		c = argv[arg_offset][0];
		if(c != '-') {
			break;
		}
		c = argv[arg_offset][1];
		switch(c) {
		case 'm':
			arg_offset++;
			loop_multipler = atoi(argv[arg_offset]);
			break;
		case 'h':
			print_usage(argv[0]);
			exit(0);
			break;
		default:
			printf("Unkown common option '%s'\n", argv[arg_offset]);
			break;
		}
		arg_offset++;
	}

	while((info = bench_enc_get_next(info)) != NULL) {
		printf("============================================================================\n");
		rc = bench_enc_run(info);
		printf("============================================================================\n\n");
	}
	if(rc != 0 && info != NULL) {
		printf(
		"============================================================================\n"
		"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
		"============================================================================\n"
		"============= failed bench: %s, rc = %d\n"
		"============================================================================\n"
		"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
		"============================================================================\n",
		info->name, rc);
	}

	cleanup_protobuf();
	fflush(stdout);

	return rc;
}

