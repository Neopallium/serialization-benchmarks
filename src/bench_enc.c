
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

#define MAX_BUFFER_SIZE 128 * 1024

#undef MAX_DOUBLE
#define MAX_DOUBLE (1.0e30)

static int loop_multipler = 100;
static int g_trials = 20;
static bool g_test = false;

#define LOOP_ENC_DEC       (20 * loop_multipler)
#define LOOP_CREATE_DELETE (20 * loop_multipler)
#define LOOP_ROUND_TRIP    (20 * loop_multipler)
#define LOOP_CREATE        (20 * loop_multipler)

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
	BenchCreateCheckDeleteFull  = 2,
	BenchEncodeDifferentObjects = 3,
	BenchEncodeSizeObject       = 4,
	BenchEncodeSameObject       = 5,
	BenchDecodeObject           = 6,
	BenchDecodeSameObject       = 7,
	BenchDecodeObjectCheckAll   = 8,
	BenchDecodeObjectCheckMedia = 9,
	BenchDecodeEncodeRoundTrip  = 10,
	BenchStatIdMax              = 11,
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
	{ 1, "Create/Check full",    "create, check & delete full object" },
	{ 0, "Encode diff",          "encode different objects" },
	{ 0, "Encode size",          "encode size object" },
	{ 0, "Encode same",          "encode same object" },
	{ 0, "Decode",               "decode object" },
	{ 0, "Decode same",          "decode same object" },
	{ 0, "Decode & Check all",   "decode object & check all fields" },
	{ 0, "Decode & Check media", "decode object & check media field" },
	{ 0, "Round Trip",           "Round trip decode <-> encode <-> delete" },
};

static BenchEncInfo *bench_enc_list = NULL;
static BenchEncInfo *bench_enc_find_msg_list(const char *msg_name) {
	BenchEncInfo *info = bench_enc_list;
	/* find existing message list. */
	while(info != NULL) {
		if(strcmp(info->msg_name, msg_name) == 0) {
			return info;
		}
		info = info->msg_next;
	}
	/* not found. */
	return NULL;
}

void bench_enc_reg(BenchEncInfo *test) {
	/* add bench test to list with the same message name. */
	BenchEncInfo *list = bench_enc_find_msg_list(test->msg_name);
	if(list) {
		/* list already started for this message. */
		test->next = list->next;
		list->next = test;
	} else {
		/* new message, start new list. */
		test->msg_next = bench_enc_list;
		bench_enc_list = test;
		test->next = NULL;
	}
	/* allocate stats/counts array for encoder. */
	test->stats = (double *)calloc(MAX_BENCH_STATS, sizeof(double));
	for(int i=0; i<MAX_BENCH_STATS; i++) test->stats[i] = MAX_DOUBLE;
	test->bytes = (size_t *)calloc(MAX_BENCH_STATS, sizeof(size_t));
	test->counts = (uint32_t *)calloc(MAX_BENCH_STATS, sizeof(uint32_t));
}

static void bench_enc_cleanup_stats() {
	BenchEncInfo *list = bench_enc_list;
	BenchEncInfo *info;

	while(list) {
		info = list;
		while(info) {
			free(info->stats);
			free(info->bytes);
			free(info->counts);
			info = info->next;
		}
		list = list->msg_next;
	}
}

static void bench_enc_dump_info() {
	BenchEncInfo *list = bench_enc_list;
	BenchEncInfo *info;

	while(list) {
		info = list;
		printf("Message: %s\n", info->msg_name);
		while(info) {
			printf("  Benchmark: %s\n", info->name);
			info = info->next;
		}
		list = list->msg_next;
	}
}

static BenchEncInfo *bench_enc_current_list = NULL;
static BenchEncInfo *bench_enc_select_msg(const char *msg_name) {
	if(msg_name != NULL) {
		bench_enc_current_list = bench_enc_find_msg_list(msg_name);
	} else {
		bench_enc_current_list = bench_enc_list;
	}
	return bench_enc_current_list;
}

BenchEncInfo *bench_enc_get_next(BenchEncInfo *info) {
	if(info == NULL) {
		return bench_enc_current_list;
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
	BenchEncInfo *info = bench_enc_current_list;
	int s;

	/* print headers. */
	printf("Units: nano seconds\n");
	printf("%10s", "");
	for(s = 0; s < MAX_BENCH_STATS; s++) {
		printf(", %12s", bench_enc_stats_info[s].name);
	}
	printf(", %12s", "Encode Size");
	printf("\n");
	info = bench_enc_current_list;
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
	info = bench_enc_current_list;
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
	info = bench_enc_current_list;
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
	if(g_test) {
		printf("%20s: %6.0f nsecs\n", bench_enc_stats_info[stat_id].name,
			((double)((secs * NSEC_PER_SEC) / count)));
	}
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

static void bench_create_full_object_check_all(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	void         *obj = NULL;
	uint32_t     n;

	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		obj = info->build(NULL);
		info->check_all(obj);
		info->free(obj);
	}
	/* bench finished */
	btimer_stop(timer);
	record_bench_stat(BenchCreateCheckDeleteFull, bench, count, 0);
}

static void bench_encode_size_object(BenchEnc *bench, uint32_t count) {
	BTimer       timer = bench->timer;
	BenchEncInfo *info = bench->info;
	void         *state = bench->state;
	void         *obj = NULL;
	size_t       len = 0;
	size_t       total_len = 0;
	uint32_t     n;

	obj = info->build(NULL);
	/* start timer. */
	btimer_start(timer);
	for(n = 0; n < count; n++) {
		len = info->enc_size(state, obj);
		total_len += len;
	}
	/* bench finished */
	btimer_stop(timer);
	/* record encode size. */
	assert(info->encode_size == 0 || info->encode_size == len);
	info->encode_size = len;
	record_bench_stat(BenchEncodeSizeObject, bench, count, total_len);
	info->free(obj);
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
	assert(info->encode_size == 0 || info->encode_size == len);
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
	assert(info->encode_size == 0 || info->encode_size == len);
	info->encode_size = len;
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

static void bench_decode_object_check_partial(BenchEnc *bench, uint32_t count) {
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
		info->check_partial(obj);
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

	printf("--------- Benchmark: %s\n", info->name);
	bench = bench_enc_new(info);

	/* test check_* functions. */
	obj = info->build(NULL);
	info->check_all(obj);
	info->free(obj);


	for(i = 0; i < g_trials; i++)
		bench_create_empty_objects(bench, LOOP_CREATE_DELETE);
	for(i = 0; i < g_trials; i++)
		bench_create_full_objects(bench, LOOP_CREATE_DELETE);
	for(i = 0; i < g_trials; i++)
		bench_create_full_object_check_all(bench, LOOP_CREATE_DELETE);

	for(i = 0; i < g_trials; i++)
		bench_encode_size_object(bench, LOOP_ENC_DEC);

	for(i = 0; i < g_trials; i++)
		bench_encode_different_objects(bench, LOOP_ENC_DEC);
	for(i = 0; i < g_trials; i++)
		bench_encode_same_object(bench, LOOP_ENC_DEC);

	for(i = 0; i < g_trials; i++)
		bench_decode_object(bench, LOOP_ENC_DEC);
	for(i = 0; i < g_trials; i++)
		bench_decode_same_object(bench, LOOP_ENC_DEC);
	for(i = 0; i < g_trials; i++)
		bench_decode_object_check_all(bench, LOOP_ENC_DEC);
	for(i = 0; i < g_trials; i++)
		bench_decode_object_check_partial(bench, LOOP_ENC_DEC);

	for(i = 0; i < g_trials; i++)
		bench_decode_encode_round_trip(bench, LOOP_ROUND_TRIP);

	bench_enc_free(bench);
	return rc;
}

static void print_usage(char *name) {
	printf("Usage: %s [-h] [-t] [-m <loop multipler>] [--dump] [<msg_name>]\n", name);
}

#define unknown_option() do { \
	printf("Unknown option '%s'\n", opt); \
	print_usage(argv[0]); \
	exit(-1); \
} while(0)

int main(int argc, char **argv) {
	BenchEncInfo *info = NULL;
	const char *msg_name = NULL;
	const char *opt;
	int arg_offset = 1;
	int rc = 0;
	char c;

	// find test name and parse common args.
	while(argc > arg_offset) {
		opt = argv[arg_offset];
		c = opt[0];
		if(c != '-') {
			break;
		}
		c = opt[1];
		switch(c) {
		case '-':
			c = opt[2];
			switch(c) {
			case 'd':
				if(strcmp("--dump", opt) != 0) {
					unknown_option();
				}
				bench_enc_dump_info();
				exit(0);
				break;
			default:
				unknown_option();
			}
			break;
		case 't':
			g_trials = 10;
			g_test = true;
			break;
		case 'm':
			arg_offset++;
			loop_multipler = atoi(argv[arg_offset]);
			break;
		case 'h':
			print_usage(argv[0]);
			exit(0);
			break;
		default:
			unknown_option();
		}
		arg_offset++;
	}
	/* select message based on name. */
	if(argc > arg_offset) {
		msg_name = argv[arg_offset];
		printf("Run benchmarks for message: %s\n", msg_name);
	}
	if(bench_enc_select_msg(msg_name) == NULL) {
		if(msg_name) {
			printf("Can't find benchmarks for msg_name='%s'\n", msg_name);
		} else {
			printf("Can't find benchmarks to run!\n");
		}
		return -1;
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

