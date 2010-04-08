/***************************************************************************
 * Copyright (C) 2007-2010 by Robert G. Jakabosky <bobby@neoawareness.com> *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>

#include <avro.h>

#include "bench_enc.h"

enum Size {
	SMALL = 0,
	LARGE = 1
};

enum Player {
	JAVA = 0,
	FLASH = 1
};

AVRO_ATOMS_HOLDER_START(media)
AVRO_ATOMS_HOLDER(bitrate)
AVRO_ATOMS_HOLDER(copyright)
AVRO_ATOMS_HOLDER(duration)
AVRO_ATOMS_HOLDER(format)
AVRO_ATOMS_HOLDER(height)
AVRO_ATOMS_HOLDER(image)
AVRO_ATOMS_HOLDER(media)
AVRO_ATOMS_HOLDER(person)
AVRO_ATOMS_HOLDER(player)
AVRO_ATOMS_HOLDER(size)
AVRO_ATOMS_HOLDER(title)
AVRO_ATOMS_HOLDER(uri)
AVRO_ATOMS_HOLDER(width)
AVRO_ATOMS_HOLDER_END(media)

AVRO_ATOMS_REG_START(media)
AVRO_ATOMS_REG(media, bitrate, bitrate)
AVRO_ATOMS_REG(media, copyright, copyright)
AVRO_ATOMS_REG(media, duration, duration)
AVRO_ATOMS_REG(media, format, format)
AVRO_ATOMS_REG(media, height, height)
AVRO_ATOMS_REG(media, image, image)
AVRO_ATOMS_REG(media, media, media)
AVRO_ATOMS_REG(media, person, person)
AVRO_ATOMS_REG(media, player, player)
AVRO_ATOMS_REG(media, size, size)
AVRO_ATOMS_REG(media, title, title)
AVRO_ATOMS_REG(media, uri, uri)
AVRO_ATOMS_REG(media, width, width)
AVRO_ATOMS_REG_END(media)

static const char *media_schema =
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

typedef struct AvroBenchState {
	avro_schema_t media;
	avro_writer_t writer;
} AvroBenchState;


static void * create_avro() {
	return avro_record("MediaContent", NULL);
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
	media = avro_record("Media", NULL);
	tmp = avro_wrapstring("http://javaone.com/keynote.mpg");
	avro_record_set(media, AVRO_ATOM(media, uri), tmp); avro_datum_decref(tmp);
	tmp = avro_wrapstring("Javaone Keynote");
	avro_record_set(media, AVRO_ATOM(media, title), tmp); avro_datum_decref(tmp);
	tmp = avro_wrapstring("video/mpg4");
	avro_record_set(media, AVRO_ATOM(media, format), tmp); avro_datum_decref(tmp);

	tmp = avro_int32(JAVA); avro_record_set(media, AVRO_ATOM(media, player), tmp); avro_datum_decref(tmp);
	tmp = avro_int64(1234567); avro_record_set(media, AVRO_ATOM(media, duration), tmp); avro_datum_decref(tmp);
	tmp = avro_int64(123); avro_record_set(media, AVRO_ATOM(media, size), tmp); avro_datum_decref(tmp);
	tmp = avro_int32(0); avro_record_set(media, AVRO_ATOM(media, height), tmp); avro_datum_decref(tmp);
	tmp = avro_int32(0); avro_record_set(media, AVRO_ATOM(media, width), tmp); avro_datum_decref(tmp);
	tmp = avro_int32(0); avro_record_set(media, AVRO_ATOM(media, bitrate), tmp); avro_datum_decref(tmp);

	tmp = avro_wrapstring("");
	avro_record_set(media, AVRO_ATOM(media, copyright), tmp); avro_datum_decref(tmp);

	/* add persons. */
	array = avro_array();
	tmp = avro_wrapstring("Bill Gates");
	avro_array_append_datum(array, tmp); avro_datum_decref(tmp);
	tmp = avro_wrapstring("Steve Jobs");
	avro_array_append_datum(array, tmp); avro_datum_decref(tmp);
	avro_record_set(media, AVRO_ATOM(media, person), array);
	avro_datum_decref(array);

	/* create image records. */
	image1 = avro_record("Image", NULL);
	tmp = avro_wrapstring("Javaone Keynote");
	avro_record_set(image1, AVRO_ATOM(media, title), tmp); avro_datum_decref(tmp);
	tmp = avro_wrapstring("http://javaone.com/keynote_large.jpg");
	avro_record_set(image1, AVRO_ATOM(media, uri), tmp); avro_datum_decref(tmp);

	tmp = avro_int32(0); avro_record_set(image1, AVRO_ATOM(media, height), tmp); avro_datum_decref(tmp);
	tmp = avro_int32(0); avro_record_set(image1, AVRO_ATOM(media, width), tmp); avro_datum_decref(tmp);
	tmp = avro_int32(LARGE); avro_record_set(image1, AVRO_ATOM(media, size), tmp); avro_datum_decref(tmp);

	image2 = avro_record("Image", NULL);
	tmp = avro_wrapstring("Javaone Keynote");
	avro_record_set(image2, AVRO_ATOM(media, title), tmp); avro_datum_decref(tmp);
	tmp = avro_wrapstring("http://javaone.com/keynote_thumbnail.jpg");
	avro_record_set(image2, AVRO_ATOM(media, uri), tmp); avro_datum_decref(tmp);

	tmp = avro_int32(0); avro_record_set(image2, AVRO_ATOM(media, height), tmp); avro_datum_decref(tmp);
	tmp = avro_int32(0); avro_record_set(image2, AVRO_ATOM(media, width), tmp); avro_datum_decref(tmp);
	tmp = avro_int32(SMALL); avro_record_set(image2, AVRO_ATOM(media, size), tmp); avro_datum_decref(tmp);

	/* add media & image records to content record. */
	array = avro_array();
	avro_array_append_datum(array, image1);
	avro_array_append_datum(array, image2);
	avro_record_set(content, AVRO_ATOM(media, image), array);
	avro_datum_decref(array);
	avro_record_set(content, AVRO_ATOM(media, media), media);

	avro_datum_decref(media);
	avro_datum_decref(image1);
	avro_datum_decref(image2);

	return obj;
}

static void free_avro(void *obj) {
	avro_datum_t content = (avro_datum_t)obj;
	avro_datum_decref(content);
}

static void check_media_avro(void *obj) {
	avro_datum_t content = (avro_datum_t)obj;
	avro_datum_t media;
	avro_datum_t array;
	avro_datum_t tmp;
	int64_t i64;
	int32_t i32;
	char *p;

	check_val(avro_record_get(content, AVRO_ATOM(media, media), &media) == 0);
	check_val(avro_record_get(media, AVRO_ATOM(media, format), &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "video/mpg4") == 0);

	check_val(avro_record_get(media, AVRO_ATOM(media, player), &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == JAVA);

	check_val(avro_record_get(media, AVRO_ATOM(media, title), &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "Javaone Keynote") == 0);

	check_val(avro_record_get(media, AVRO_ATOM(media, uri), &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "http://javaone.com/keynote.mpg") == 0);

	check_val(avro_record_get(media, AVRO_ATOM(media, duration), &tmp) == 0);
	avro_int64_get(tmp, &i64);
	check_val(i64 == 1234567);

	check_val(avro_record_get(media, AVRO_ATOM(media, size), &tmp) == 0);
	avro_int64_get(tmp, &i64);
	check_val(i64 == 123);

	check_val(avro_record_get(media, AVRO_ATOM(media, height), &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 0);

	check_val(avro_record_get(media, AVRO_ATOM(media, width), &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 0);

	check_val(avro_record_get(media, AVRO_ATOM(media, bitrate), &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 0);

	check_val(avro_record_get(media, AVRO_ATOM(media, person), &array) == 0);
#ifdef HAVE_avro_array_get
	check_val(avro_array_get(array, 0, &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "Bill Gates") == 0);
	check_val(avro_array_get(array, 1, &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "Steve Jobs") == 0);
#endif

	check_val(avro_record_get(media, AVRO_ATOM(media, copyright), &tmp) == 0);
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

	check_media_avro(content);

	check_val(avro_record_get(content, AVRO_ATOM(media, image), &array) == 0);
#ifdef HAVE_avro_array_get
	check_val(avro_array_get(array, 0, &image) == 0);
	check_val(avro_record_get(image, AVRO_ATOM(media, height), &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 0);

	check_val(avro_record_get(image, AVRO_ATOM(media, title), &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "Javaone Keynote") == 0);

	check_val(avro_record_get(image, AVRO_ATOM(media, uri), &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "http://javaone.com/keynote_large.jpg") == 0);

	check_val(avro_record_get(image, AVRO_ATOM(media, width), &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 0);

	check_val(avro_record_get(image, AVRO_ATOM(media, size), &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == LARGE);

	check_val(avro_array_get(array, 1, &image) == 0);
	check_val(avro_record_get(image, AVRO_ATOM(media, height), &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 0);

	check_val(avro_record_get(image, AVRO_ATOM(media, title), &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "Javaone Keynote") == 0);

	check_val(avro_record_get(image, AVRO_ATOM(media, uri), &tmp) == 0);
	avro_string_get(tmp, &p);
	check_val(strcmp(p, "http://javaone.com/keynote_thumbnail.jpg") == 0);

	check_val(avro_record_get(image, AVRO_ATOM(media, width), &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == 0);

	check_val(avro_record_get(image, AVRO_ATOM(media, size), &tmp) == 0);
	avro_int32_get(tmp, &i32);
	check_val(i32 == SMALL);
#endif

}

static void * init_avro(char *buf, size_t len) {
	AvroBenchState *bstate = NULL;
	avro_schema_error_t error;

	bstate = (AvroBenchState *)malloc(sizeof(AvroBenchState));

	avro_init();

	/* parser media schema. */
	if(avro_schema_from_json(media_schema, sizeof(media_schema), &(bstate->media), &error)) {
		printf("Failed to parse media schema\n");
	}
	/* setup writer. */
	bstate->writer = avro_writer_memory(buf, len);

	return bstate;
}

static void cleanup_avro(void *state) {
	AvroBenchState *bstate = (AvroBenchState *)state;

	/* free writer. */
	avro_writer_free(bstate->writer);

	/* release media schema. */
	avro_schema_decref(bstate->media);

	avro_shutdown();

	free(bstate);
}

static size_t encode_avro(void *state, void *obj, char *buf, size_t len) {
	AvroBenchState *bstate = (AvroBenchState *)state;
	avro_datum_t datum = (avro_datum_t)obj;
	int rc = 0;
	(void)buf;
	(void)len;

	avro_writer_reset(bstate->writer);
	/* encode object. */
	if((rc = avro_write_data(bstate->writer, bstate->media, datum))) {
		printf("avro_write_data: failed rc=%d\n", rc);
		return 0;
	}
	avro_writer_flush(bstate->writer);
	return avro_writer_tell(bstate->writer);
}

static void * decode_avro(void *state, void *obj, char *buf, size_t len) {
	AvroBenchState *bstate = (AvroBenchState *)state;
	avro_reader_t reader;
	avro_datum_t datum;
	int rc;

	/* can't re-use old object. */
	if(obj) free_avro(obj);

	reader = avro_reader_memory(buf, len);

	/* decode object. */
	if((rc = avro_read_data(reader, bstate->media, NULL, &datum))) {
		printf("avro_read_data: failed rc=%d\n", rc);
	}
	avro_reader_free(reader);
	return datum;
}

BENCH_ENC_REG(avro, "Benchmark: avro system.")

