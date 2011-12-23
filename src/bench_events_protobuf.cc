/***************************************************************************
 * Copyright (C) 2007-2010 by Robert G. Jakabosky <bobby@neoawareness.com> *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>

#include <google/protobuf/stubs/common.h>
#include "events.pb.h"

using namespace serializers::protobuf;

extern "C" {
#include "bench_enc.h"

static void * create_protobuf() {
	return new IOEvent();
}

static void * build_protobuf(void *obj) {
	IOEvent *event;

	/* create IOEvent object. */
	if(obj == NULL) {
		obj = new IOEvent();
	}
	event = (IOEvent *)obj;

	/* create IO Event record. */
	event->set_id(123);
	event->set_fd(1000000);
	event->set_events(10);

	return obj;
}

static void free_protobuf(void *obj) {
	IOEvent *event = (IOEvent *)obj;
	delete event;
}

static void check_part_protobuf(void *obj) {
	IOEvent *event = (IOEvent *)obj;

	check_val(event->id() == 123);
	check_val(event->fd() == 1000000);
	check_val(event->events() == 10);
}

static void check_all_protobuf(void *obj) {
	IOEvent *event = (IOEvent *)obj;

	check_part_protobuf(event);
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
	IOEvent *event = (IOEvent *)obj;
	(void)state;

	return event->ByteSize();
}

static size_t encode_protobuf(void *state, void *obj, char *buf, size_t buflen) {
	IOEvent *event = (IOEvent *)obj;
	int len;
	uint8_t *p;
	uint8_t *p_end;
	(void)state;

	len = event->ByteSize();
	if(len > (int)buflen) return 0;
	p = (uint8_t *)buf;
	p_end = event->SerializeWithCachedSizesToArray(p);
	if((p_end - p) < len) {
		printf("Encoder failed: encode len = %d, obj len = %d\n", (int)(p_end - p), len);
		return 0;
	}
	return len;
}

static void * decode_protobuf(void *state, void *obj, char *buf, size_t len) {
	IOEvent *event;
	(void)state;

	if(obj) {
		event = (IOEvent *)obj;
		event->Clear();
	} else {
		event = new IOEvent();
	}
	if(!event->ParseFromArray(buf, len)) {
		printf("Decoder failed: \n");
		free_protobuf(event);
		event = NULL;
	}
	return event;
}

BENCH_ENC_REG(protobuf, events, "Benchmark: protobuf system.")

}// end: extern "C"

