/***************************************************************************
 * Copyright (C) 2007-2010 by Robert G. Jakabosky <bobby@neoawareness.com> *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>

#include <google/protobuf/stubs/common.h>
#include "media.pb.h"

using namespace serializers::protobuf;

extern "C" {
#include "bench_enc.h"

static void * create_protobuf() {
	return new MediaContent();
}

static void * build_protobuf(void *obj) {
	MediaContent *content;
	Media *media;
	Image *image1;
	Image *image2;

	/* create MediaContent object. */
	if(obj == NULL) {
		obj = new MediaContent();
	}
	content = (MediaContent *)obj;

	/* create media record. */
	media = content->mutable_media();
	media->set_format("video/mpg4");
	media->set_player(Media_Player_JAVA);
	media->set_title("Javaone Keynote");
	media->set_uri("http://javaone.com/keynote.mpg");
	media->set_duration(18000000);
	media->set_size(58982400);
	media->set_height(640);
	media->set_width(480);
	media->set_bitrate(262144);
	/* add persons. */
	media->add_person("Bill Gates");
	media->add_person("Steve Jobs");

	/* create image records. */
	image1 = content->add_image();
	image1->set_height(768);
	image1->set_title("Javaone Keynote");
	image1->set_uri("http://javaone.com/keynote_large.jpg");
	image1->set_width(1024);
	image1->set_size(Image_Size_LARGE);

	image2 = content->add_image();
	image2->set_height(240);
	image2->set_title("Javaone Keynote");
	image2->set_uri("http://javaone.com/keynote_thumbnail.jpg");
	image2->set_width(320);
	image2->set_size(Image_Size_SMALL);

	return obj;
}

static void free_protobuf(void *obj) {
	MediaContent *content = (MediaContent *)obj;
	delete content;
}

static void check_part_protobuf(void *obj) {
	MediaContent *content = (MediaContent *)obj;
	Media media;

	media = content->media();
	check_val(media.format() == "video/mpg4");
	check_val(media.player() == Media_Player_JAVA);
	check_val(media.title() == "Javaone Keynote");
	check_val(media.uri() == "http://javaone.com/keynote.mpg");
	check_val(media.duration() == 18000000);
	check_val(media.size() == 58982400);
	check_val(media.height() == 640);
	check_val(media.width() == 480);
	check_val(media.bitrate() == 262144);
	/* add persons. */
	check_val(media.person(0) == "Bill Gates");
	check_val(media.person(1) == "Steve Jobs");
}

static void check_all_protobuf(void *obj) {
	MediaContent *content = (MediaContent *)obj;
	Image image;

	check_part_protobuf(content);

	image = content->image(0);
	check_val(image.height() == 768);
	check_val(image.title() == "Javaone Keynote");
	check_val(image.uri() == "http://javaone.com/keynote_large.jpg");
	check_val(image.width() == 1024);
	check_val(image.size() == Image_Size_LARGE);

	image = content->image(1);
	check_val(image.height() == 240);
	check_val(image.title() == "Javaone Keynote");
	check_val(image.uri() == "http://javaone.com/keynote_thumbnail.jpg");
	check_val(image.width() == 320);
	check_val(image.size() == Image_Size_SMALL);

}

static void * init_protobuf(char *buf, size_t len) {
	(void)buf;
	(void)len;
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	return NULL;
}

static void cleanup_protobuf(void *state) {
	(void)state;
#ifdef BENCH_PROTOBUF
	google::protobuf::ShutdownProtobufLibrary();
#endif
}

static size_t enc_size_protobuf(void *state, void *obj) {
	MediaContent *content = (MediaContent *)obj;
	(void)state;

	return content->ByteSize();
}

static size_t encode_protobuf(void *state, void *obj, char *buf, size_t buflen) {
	MediaContent *content = (MediaContent *)obj;
	int len;
	uint8_t *p;
	uint8_t *p_end;
	(void)state;

	len = content->ByteSize();
	if(len > (int)buflen) return 0;
	p = (uint8_t *)buf;
	p_end = content->SerializeWithCachedSizesToArray(p);
	if((p_end - p) < len) {
		printf("Encoder failed: encode len = %d, obj len = %d\n", (int)(p_end - p), len);
		return 0;
	}
	return len;
}

static void * decode_protobuf(void *state, void *obj, char *buf, size_t len) {
	MediaContent *content;
	(void)state;

	if(obj) {
		content = (MediaContent *)obj;
		content->Clear();
	} else {
		content = new MediaContent();
	}
	if(!content->ParseFromArray(buf, len)) {
		printf("Decoder failed: \n");
		free_protobuf(content);
		content = NULL;
	}
	return content;
}

BENCH_ENC_REG(protobuf, media, "Benchmark: protobuf system.")

}// end: extern "C"

