/***************************************************************************
 * Copyright (C) 2007-2010 by Robert G. Jakabosky <bobby@neoawareness.com> *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>

#include <avro.h>

#include "bench_enc.h"

#define VERBOSE 0

enum Size {
	SMALL = 0,
	LARGE = 1
};

enum Player {
	JAVA = 0,
	FLASH = 1
};

avro_schema_t media_content_sch;
avro_schema_t image_array_sch;
avro_schema_t image_sch;
avro_schema_t media_sch;
avro_schema_t person_array_sch;

static const char *media_content_schema =
"{\"type\": \"record\", \"name\":\"MediaContent\", \"namespace\": \"serializers.avro.specific\","
"  \"fields\": ["
"    {\"name\": \"image\", \"type\": {\"type\": \"array\", "
"      \"items\": {\"type\": \"record\", \"name\":\"Image\","
"        \"fields\": ["
"          {\"name\": \"uri\", \"type\": \"string\"},"
"          {\"name\": \"title\", \"type\": \"string\"},"
"          {\"name\": \"width\", \"type\": \"int\"},"
"          {\"name\": \"height\", \"type\": \"int\"},"
"          {\"name\": \"size\", \"type\": \"int\"}"
"        ]"
"      }"
"    }},"
"    {\"name\": \"media\", "
"      \"type\": {\"type\": \"record\", \"name\":\"Media\","
"        \"fields\": ["
"          {\"name\": \"uri\", \"type\": \"string\"},"
"          {\"name\": \"title\", \"type\": \"string\"},"
"          {\"name\": \"width\", \"type\": \"int\"},"
"          {\"name\": \"height\", \"type\": \"int\"},"
"          {\"name\": \"format\", \"type\": \"string\"},"
"          {\"name\": \"duration\", \"type\": \"long\"},"
"          {\"name\": \"size\", \"type\": \"long\"},"
"          {\"name\": \"bitrate\", \"type\": \"int\"},"
"          {\"name\": \"person\", \"type\": {\"type\": \"array\", \"items\": \"string\"}},"
"          {\"name\": \"player\", \"type\": \"int\"},"
"          {\"name\": \"copyright\", \"type\": \"string\"}"
"        ]"
"      }"
"    }"
"  ]"
"}";

#if VERBOSE
#define DUMP_SCHEMA_BUFLEN 64 * 1024
static void dump_avro_schema(const avro_schema_t schema, const char *prefix) {
	avro_writer_t out;
	char buf[DUMP_SCHEMA_BUFLEN];
	int64_t rc;

	if(schema == NULL) {
		fprintf(stdout,"%s: NULL schema\n", prefix);
		return;
	}
	fprintf(stdout,"%s: ", prefix);
	out = avro_writer_memory(buf, DUMP_SCHEMA_BUFLEN);
	avro_schema_to_json(schema, out);
	rc = avro_writer_tell(out);
	if(rc > 0) {
		if(fwrite(buf, 1, rc, stdout) == 0) {
			perror("dump_avro_scheme: fwrite()");
		}
	}
	avro_writer_free(out);
	fprintf(stdout,"\n");
	fflush(stdout);
}
#endif

typedef struct AvroBenchState {
	avro_writer_t writer;
} AvroBenchState;


static void * create_avro() {
	return avro_record(media_content_sch);
}

static void * build_avro(void *obj) {
	avro_datum_t content;
	avro_datum_t media;
	avro_datum_t image1;
	avro_datum_t image2;
	avro_datum_t array;
	avro_datum_t tmp;

	/* create MediaContent record. */
	if(obj == NULL) {
		obj = create_avro();
	}
	content = (avro_datum_t)obj;

	/* create media record. */
	media = avro_record(media_sch);
	tmp = avro_givestring("http://javaone.com/keynote.mpg",NULL);
	avro_record_set(media, "uri", tmp); avro_datum_decref(tmp);
	tmp = avro_givestring("Javaone Keynote",NULL);
	avro_record_set(media, "title", tmp); avro_datum_decref(tmp);
	tmp = avro_givestring("video/mpg4",NULL);
	avro_record_set(media, "format", tmp); avro_datum_decref(tmp);

	tmp = avro_int32(JAVA); avro_record_set(media, "player", tmp); avro_datum_decref(tmp);
	tmp = avro_int64(18000000); avro_record_set(media, "duration", tmp); avro_datum_decref(tmp);
	tmp = avro_int64(58982400); avro_record_set(media, "size", tmp); avro_datum_decref(tmp);
	tmp = avro_int32(480); avro_record_set(media, "height", tmp); avro_datum_decref(tmp);
	tmp = avro_int32(640); avro_record_set(media, "width", tmp); avro_datum_decref(tmp);
	tmp = avro_int32(262144); avro_record_set(media, "bitrate", tmp); avro_datum_decref(tmp);

	tmp = avro_givestring("",NULL);
	avro_record_set(media, "copyright", tmp); avro_datum_decref(tmp);

	/* add persons. */
	array = avro_array(person_array_sch);
	tmp = avro_givestring("Bill Gates",NULL);
	avro_array_append_datum(array, tmp); avro_datum_decref(tmp);
	tmp = avro_givestring("Steve Jobs",NULL);
	avro_array_append_datum(array, tmp); avro_datum_decref(tmp);
	avro_record_set(media, "person", array);
	avro_datum_decref(array);

	/* create image records. */
	image1 = avro_record(image_sch);
	tmp = avro_givestring("Javaone Keynote",NULL);
	avro_record_set(image1, "title", tmp); avro_datum_decref(tmp);
	tmp = avro_givestring("http://javaone.com/keynote_large.jpg",NULL);
	avro_record_set(image1, "uri", tmp); avro_datum_decref(tmp);

	tmp = avro_int32(768); avro_record_set(image1, "height", tmp); avro_datum_decref(tmp);
	tmp = avro_int32(1024); avro_record_set(image1, "width", tmp); avro_datum_decref(tmp);
	tmp = avro_int32(LARGE); avro_record_set(image1, "size", tmp); avro_datum_decref(tmp);

	image2 = avro_record(image_sch);
	tmp = avro_givestring("Javaone Keynote",NULL);
	avro_record_set(image2, "title", tmp); avro_datum_decref(tmp);
	tmp = avro_givestring("http://javaone.com/keynote_thumbnail.jpg",NULL);
	avro_record_set(image2, "uri", tmp); avro_datum_decref(tmp);

	tmp = avro_int32(240); avro_record_set(image2, "height", tmp); avro_datum_decref(tmp);
	tmp = avro_int32(320); avro_record_set(image2, "width", tmp); avro_datum_decref(tmp);
	tmp = avro_int32(SMALL); avro_record_set(image2, "size", tmp); avro_datum_decref(tmp);

	/* add media & image records to content record. */
	array = avro_array(image_array_sch);
	avro_array_append_datum(array, image1);
	avro_array_append_datum(array, image2);
	avro_record_set(content, "image", array);
	avro_datum_decref(array);
	avro_record_set(content, "media", media);

	avro_datum_decref(media);
	avro_datum_decref(image1);
	avro_datum_decref(image2);

	return obj;
}

static void free_avro(void *obj) {
	avro_datum_t content = (avro_datum_t)obj;
	avro_datum_decref(content);
}

static void check_part_avro(void *obj) {
	avro_datum_t content = (avro_datum_t)obj;
	avro_datum_t media;
	avro_datum_t array;
	avro_datum_t tmp;
	int64_t i64;
	int32_t i32;
	char *p;

	check_val(avro_record_get(content, "media", &media) == 0);
	check_val(avro_record_get(media, "format", &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "video/mpg4") == 0);

	check_val(avro_record_get(media, "player", &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == JAVA);

	check_val(avro_record_get(media, "title", &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "Javaone Keynote") == 0);

	check_val(avro_record_get(media, "uri", &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "http://javaone.com/keynote.mpg") == 0);

	check_val(avro_record_get(media, "duration", &tmp) == 0);
	avro_int64_get(tmp, &i64);
	check_val(i64 == 18000000);

	check_val(avro_record_get(media, "size", &tmp) == 0);
	avro_int64_get(tmp, &i64);
	check_val(i64 == 58982400);

	check_val(avro_record_get(media, "height", &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 480);

	check_val(avro_record_get(media, "width", &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 640);

	check_val(avro_record_get(media, "bitrate", &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 262144);

	check_val(avro_record_get(media, "person", &array) == 0);
#ifdef HAVE_avro_array_get
	check_val(avro_array_get(array, 0, &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "Bill Gates") == 0);
	check_val(avro_array_get(array, 1, &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "Steve Jobs") == 0);
#endif

	check_val(avro_record_get(media, "copyright", &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "") == 0);

}

static void check_all_avro(void *obj) {
	avro_datum_t content = (avro_datum_t)obj;
	avro_datum_t array;
#ifdef HAVE_avro_array_get
	avro_datum_t image;
	avro_datum_t tmp;
	int32_t i32;
	char *p;
#endif

	check_part_avro(content);

	check_val(avro_record_get(content, "image", &array) == 0);
#ifdef HAVE_avro_array_get
	check_val(avro_array_get(array, 0, &image) == 0);
	check_val(avro_record_get(image, "height", &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 768);

	check_val(avro_record_get(image, "title", &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "Javaone Keynote") == 0);

	check_val(avro_record_get(image, "uri", &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "http://javaone.com/keynote_large.jpg") == 0);

	check_val(avro_record_get(image, "width", &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 1024);

	check_val(avro_record_get(image, "size", &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == LARGE);

	check_val(avro_array_get(array, 1, &image) == 0);
	check_val(avro_record_get(image, "height", &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 240);

	check_val(avro_record_get(image, "title", &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "Javaone Keynote") == 0);

	check_val(avro_record_get(image, "uri", &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "http://javaone.com/keynote_thumbnail.jpg") == 0);

	check_val(avro_record_get(image, "width", &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 320);

	check_val(avro_record_get(image, "size", &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == SMALL);
#endif

}

static void * init_avro(char *buf, size_t len) {
	AvroBenchState *bstate = NULL;
	avro_schema_error_t error;

	bstate = (AvroBenchState *)malloc(sizeof(AvroBenchState));

	/* parser media schema. */
	if(avro_schema_from_json(media_content_schema, sizeof(media_content_schema),
			&(media_content_sch), &error)) {
		printf("Failed to parse media schema\n");
	}

	image_array_sch = avro_schema_record_field_get(media_content_sch, "image");
	image_sch = avro_schema_array_items(image_array_sch);
	media_sch = avro_schema_record_field_get(media_content_sch, "media");
	person_array_sch = avro_schema_record_field_get(media_sch, "person");
#if VERBOSE
	dump_avro_schema(media_content_sch, "\nFull Schema");
	dump_avro_schema(image_array_sch, "\nImage array");
	dump_avro_schema(image_sch, "\nImage");
	dump_avro_schema(media_sch, "\nMedia");
	dump_avro_schema(person_array_sch, "\nPerson");
	fflush(stdout);
#endif
	/* setup writer. */
	bstate->writer = avro_writer_memory(buf, len);

	return bstate;
}

static void cleanup_avro(void *state) {
	AvroBenchState *bstate = (AvroBenchState *)state;

	/* free writer. */
	avro_writer_free(bstate->writer);

	/* release media schema. */
	avro_schema_decref(media_content_sch);

	free(bstate);
}

static size_t enc_size_avro(void *state, void *obj) {
	(void)state;
	(void)obj;
	return 0;
}

static size_t encode_avro(void *state, void *obj, char *buf, size_t len) {
	AvroBenchState *bstate = (AvroBenchState *)state;
	avro_datum_t datum = (avro_datum_t)obj;
	int rc = 0;
	(void)buf;
	(void)len;

	avro_writer_reset(bstate->writer);
	/* encode object. */
	if((rc = avro_write_data(bstate->writer, media_content_sch, datum))) {
		printf("avro_write_data: failed rc=%d\n", rc);
		return 0;
	}
	avro_writer_flush(bstate->writer);
	return avro_writer_tell(bstate->writer);
}

static void * decode_avro(void *state, void *obj, char *buf, size_t len) {
	avro_reader_t reader;
	avro_datum_t datum;
	int rc;
	(void)state;

	/* can't re-use old object. */
	if(obj) free_avro(obj);

	reader = avro_reader_memory(buf, len);

	/* decode object. */
	if((rc = avro_read_data(reader, media_content_sch, NULL, &datum))) {
		printf("avro_read_data: failed rc=%d\n", rc);
	}
	avro_reader_free(reader);
	return datum;
}

BENCH_ENC_REG(avro, media, "Benchmark: avro system.")

