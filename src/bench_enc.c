
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

#define TEST 0

#define MAX_BUFFER_SIZE 128 * 1024

#undef MAX_DOUBLE
#define MAX_DOUBLE (1.0e30)

static int loop_multipler = 100;

#define LOOP_ENC_DEC       (20 * loop_multipler)
#define LOOP_CREATE_DELETE (20 * loop_multipler)
#define LOOP_ROUND_TRIP    (20 * loop_multipler)
#define LOOP_CREATE        (20 * loop_multipler)
#if TEST
#define TRIALS 1
#else
#define TRIALS 20
#endif

#include "btimer.h"

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

typedef enum {
	BenchCreateAndDeleteEmpty   = 0,
	BenchCreateAndDeleteFull    = 1,
	BenchEncodeDifferentObjects = 2,
	BenchEncodeSameObject       = 3,
	BenchDecodeObject           = 4,
	BenchDecodeSameObject       = 5,
	BenchDecodeObjectCheckAll   = 6,
	BenchDecodeObjectCheckMedia = 7,
	BenchDecodeEncodeRoundTrip  = 8,
	BenchStatIdMax              = 9,
} BenchStatID;
#define MAX_BENCH_STATS BenchStatIdMax

typedef struct BenchStatInfo {
	bool  is_create; /**< is create & delete benchmark. */
	char  *name;     /**< short description. */
	char  *desc;     /**< long description. */
} BenchStatInfo;

static BenchStatInfo bench_enc_stats_info[] = {
	{ 1, "Create empty",         "create & delete empty object" },
	{ 1, "Create full",          "create & delete full object" },
	{ 0, "Encode diff",          "encode different objects" },
	{ 0, "Encode same",          "encode same object" },
	{ 0, "Decode",               "decode object" },
	{ 0, "Decode same",          "decode same object" },
	{ 0, "Decode & Check all",   "decode object & check all fields" },
	{ 0, "Decode & Check media", "decode object & check media field" },
	{ 0, "Round Trip",           "Round trip decode <-> encode <-> delete" },
};

static BenchEncInfo *bench_enc_list = NULL;
void bench_enc_reg(BenchEncInfo *test) {
	test->next = bench_enc_list;
	bench_enc_list = test;
	/* allocate stats/counts array for encoder. */
	test->stats = (double *)calloc(MAX_BENCH_STATS, sizeof(double));
	for(int i=0; i<MAX_BENCH_STATS; i++) test->stats[i] = MAX_DOUBLE;
	test->bytes = (size_t *)calloc(MAX_BENCH_STATS, sizeof(size_t));
	test->counts = (uint32_t *)calloc(MAX_BENCH_STATS, sizeof(uint32_t));
}

static void bench_enc_cleanup_stats() {
	BenchEncInfo *info = bench_enc_list;
	while(info) {
		free(info->stats);
		free(info->bytes);
		free(info->counts);
		info = info->next;
	}
}

BenchEncInfo *bench_enc_get_next(BenchEncInfo *info) {
	if(info == NULL) {
		return bench_enc_list;
	}
	return info->next;
}

static int stat_width(int id) {
	int width = 12;
	int name_len = strlen(bench_enc_stats_info[id].name);
	return (width < name_len) ? name_len : width;
}

/* print stats table. */
static void bench_enc_print_stats() {
	BenchEncInfo *info = bench_enc_list;
	int s;

	/* print headers. */
	printf("Units: nano seconds\n");
	printf("%10s", "");
	for(s = 0; s < MAX_BENCH_STATS; s++) {
		printf(", %12s", bench_enc_stats_info[s].name);
	}
	printf(", %12s", "Encode Size");
	printf("\n");
	info = bench_enc_list;
	while(info) {
		printf("%-10s", info->name);
		for(s = 0; s < MAX_BENCH_STATS; s++) {
			printf(", %*.0f", stat_width(s),
				((double)((info->stats[s] * NSEC_PER_SEC) / info->counts[s])));
		}
		printf(", %12.0f", ((double)info->encode_size));
		printf("\n");
		info = info->next;
	}
	printf("\n");
#if 1
	/* print headers. */
	printf("Units: MBytes per second.\n");
	printf("%10s", "");
	for(s = 0; s < MAX_BENCH_STATS; s++) {
		if(bench_enc_stats_info[s].is_create) continue;
		printf(", %12s", bench_enc_stats_info[s].name);
	}
	printf("\n");
	info = bench_enc_list;
	while(info) {
		printf("%-10s", info->name);
		for(s = 0; s < MAX_BENCH_STATS; s++) {
			if(bench_enc_stats_info[s].is_create) continue;
			printf(", %*.3f", stat_width(s), ((double)info->bytes[s]) / info->stats[s] / (1024 * 1024));
		}
		printf("\n");
		info = info->next;
	}
#endif
	printf("\n");
#if 1
	/* print headers. */
	printf("Units: Objects per second.\n");
	printf("%10s", "");
	for(s = 0; s < MAX_BENCH_STATS; s++) {
		printf(", %12s", bench_enc_stats_info[s].name);
	}
	printf("\n");
	info = bench_enc_list;
	while(info) {
		printf("%-10s", info->name);
		for(s = 0; s < MAX_BENCH_STATS; s++) {
			printf(", %*.0f", stat_width(s), ((double)info->counts[s]) / info->stats[s]);
		}
		printf("\n");
		info = info->next;
	}
#endif
}

static void record_bench_stat(BenchStatID stat_id, BenchEnc *bench, uint32_t count, size_t bytes) {
	BenchEncInfo *info = bench->info;
	double secs;

	if(stat_id > MAX_BENCH_STATS) return;

	secs = btimer_elapsed(bench->timer);
	/* only keep the minimum of each round. */
	if(info->stats[stat_id] > secs) {
		info->stats[stat_id] = secs;
		info->bytes[stat_id] = bytes;
		info->counts[stat_id] = count;
	}
#if TEST
	printf("%20s: %6.0f nsecs\n", bench_enc_stats_info[stat_id].name,
		((double)((secs * NSEC_PER_SEC) / count)));
#endif
}

static void bench_create_empty_objects(BenchEnc *bench, uint32_t count) {
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
	record_bench_stat(BenchCreateAndDeleteEmpty, bench, count, 0);
}

static void bench_create_full_objects(BenchEnc *bench, uint32_t count) {
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
	record_bench_stat(BenchCreateAndDeleteFull, bench, count, 0);
}

static void bench_encode_different_objects(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	size_t       total_len = 0;
	uint32_t     n;

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->build(NULL);
		len = info->encode(state, obj, buffer, buflen);
		total_len += len;
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	/* record encode size. */
	info->encode_size = len;
	record_bench_stat(BenchEncodeDifferentObjects, bench, count, total_len);
}

static void bench_encode_same_object(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	size_t       total_len = 0;
	uint32_t     n;

	obj = info->build(NULL);
	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		len = info->encode(state, obj, buffer, buflen);
		total_len += len;
	}
	/* bench finished */
	btimer_stop(timer);
	record_bench_stat(BenchEncodeSameObject, bench, count, total_len);
	info->free(obj);
}

static void bench_decode_object(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	size_t       total_len = 0;
	uint32_t     n;

	/* encode object. */
	obj = info->build(NULL);
	len = info->encode(state, obj, buffer, buflen);
	info->free(obj);

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->decode(state, NULL, buffer, len);
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	total_len = len * count;
	record_bench_stat(BenchDecodeObject, bench, count, total_len);
}

static void bench_decode_same_object(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	size_t       total_len = 0;
	uint32_t     n;

	/* encode object. */
	obj = info->build(NULL);
	len = info->encode(state, obj, buffer, buflen);
	info->free(obj);
	obj = NULL;

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->decode(state, obj, buffer, len);
	}
	info->free(obj);
	/* bench finished */
	btimer_stop(timer);
	total_len = len * count;
	record_bench_stat(BenchDecodeSameObject, bench, count, total_len);
}

static void bench_decode_object_check_all(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	size_t       total_len = 0;
	uint32_t     n;

	/* encode object. */
	obj = info->build(NULL);
	len = info->encode(state, obj, buffer, buflen);
	info->free(obj);

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->decode(state, NULL, buffer, len);
		info->check_all(obj);
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	total_len = len * count;
	record_bench_stat(BenchDecodeObjectCheckAll, bench, count, total_len);
}

static void bench_decode_object_check_media(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	size_t       total_len = 0;
	uint32_t     n;

	/* encode object. */
	obj = info->build(NULL);
	len = info->encode(state, obj, buffer, buflen);
	info->free(obj);

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->decode(state, NULL, buffer, len);
		info->check_media(obj);
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	total_len = len * count;
	record_bench_stat(BenchDecodeObjectCheckMedia, bench, count, total_len);
}

static void bench_decode_encode_round_trip(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	char         *buffer = bench->buffer;
	size_t       buflen = bench->len;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	size_t       total_len = 0;
	uint32_t     n;

	/* encode object. */
	obj = info->build(NULL);
	len = info->encode(state, obj, buffer, buflen);
	info->free(obj);

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->decode(state, NULL, buffer, len);
		total_len += len;
		len = info->encode(state, obj, buffer, buflen);
		total_len += len;
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	record_bench_stat(BenchDecodeEncodeRoundTrip, bench, count, total_len);
}

static int bench_enc_run(BenchEncInfo *info) {
	BenchEnc *bench = NULL;
	int      rc = 0;
	int i;
	void *obj;

	printf("Benchmark: %s\n", info->name);
	bench = bench_enc_new(info);

	/* test check_* functions. */
	obj = info->build(NULL);
	info->check_all(obj);
	info->free(obj);

	for(i = 0; i < TRIALS; i++)
		bench_create_empty_objects(bench, LOOP_CREATE_DELETE);
	for(i = 0; i < TRIALS; i++)
		bench_create_full_objects(bench, LOOP_CREATE_DELETE);

	for(i = 0; i < TRIALS; i++)
		bench_encode_different_objects(bench, LOOP_ENC_DEC);
	for(i = 0; i < TRIALS; i++)
		bench_encode_same_object(bench, LOOP_ENC_DEC);

	for(i = 0; i < TRIALS; i++)
		bench_decode_object(bench, LOOP_ENC_DEC);
	for(i = 0; i < TRIALS; i++)
		bench_decode_same_object(bench, LOOP_ENC_DEC);
	for(i = 0; i < TRIALS; i++)
		bench_decode_object_check_all(bench, LOOP_ENC_DEC);
	for(i = 0; i < TRIALS; i++)
		bench_decode_object_check_media(bench, LOOP_ENC_DEC);

	for(i = 0; i < TRIALS; i++)
		bench_decode_encode_round_trip(bench, LOOP_ROUND_TRIP);

	bench_enc_free(bench);
	return rc;
}

static void print_usage(char *name) {
	printf("Usage: %s [--dump]\n", name);
}

int main(int argc, char **argv) {
	BenchEncInfo *info = NULL;
	int arg_offset = 1;
	int rc = 0;
	char c;

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
			print_usage(argv[0]);
			break;
		}
		arg_offset++;
	}

	while((info = bench_enc_get_next(info)) != NULL) {
		rc = bench_enc_run(info);
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
	} else {
		bench_enc_print_stats();
	}

	bench_enc_cleanup_stats();
	fflush(stdout);

	return rc;
}

