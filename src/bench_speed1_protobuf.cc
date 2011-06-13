/***************************************************************************
 * Copyright (C) 2007-2010 by Robert G. Jakabosky <bobby@neoawareness.com> *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>

#include <google/protobuf/stubs/common.h>
#include "speed.pb.h"

using namespace benchmarks;

extern "C" {
#include "bench_enc.h"

static void * create_protobuf() {
	return new SpeedMessage1();
}

static void * build_protobuf(void *obj) {
	SpeedMessage1 *msg1;
	SpeedMessage1SubMessage *field15;

	/* create SpeedMessage1 object. */
	if(obj == NULL) {
		obj = new SpeedMessage1();
	}
	msg1 = (SpeedMessage1 *)obj;

	/* create main message */
	msg1->set_field1("");
	msg1->set_field2(8);
	msg1->set_field3(2066379);
	msg1->set_field4("3K+6)#");
	msg1->set_field9("10)2uiSuoXL1^)v}icF@>P(j<t#~tz\\lg??S&(<hr7EVs\'l{\'5`Gohc_(=t eS s{_I?iCwaG]L\'*Pu5(&w_:4{~Z");
	msg1->set_field12(true);
	msg1->set_field13(false);
	msg1->set_field14(true);

 	/* create sub message. */
	field15 = msg1->mutable_field15();
		field15->set_field1(25);
		field15->set_field2(36);
		field15->set_field15("\"?6PY4]L2c<}~2;\\TVF_w^[@YfbIc*v/N+Z-oYuaWZr4C;5ib|*s@RCBbuvrQ3g(k,N");
		field15->set_field21(2813090458170031956);
		field15->set_field22(38);
		field15->set_field23(true);

	msg1->set_field17(false);
	msg1->set_field18("{=Qwfe~#n{");
	msg1->set_field67(1591432);
	msg1->set_field100(31);

	return obj;
}

static void free_protobuf(void *obj) {
	SpeedMessage1 *msg1 = (SpeedMessage1 *)obj;
	delete msg1;
}

static void check_part_protobuf(void *obj) {
	SpeedMessage1 *msg1 = (SpeedMessage1 *)obj;
	SpeedMessage1SubMessage field15;

	field15 = msg1->field15();
	check_val(field15.field1() == 25);
	check_val(field15.field2() == 36);
	check_val(field15.field15() == "\"?6PY4]L2c<}~2;\\TVF_w^[@YfbIc*v/N+Z-oYuaWZr4C;5ib|*s@RCBbuvrQ3g(k,N");
	check_val(field15.field21() == 2813090458170031956);
	check_val(field15.field22() == 38);
	check_val(field15.field23() == true);
}

static void check_all_protobuf(void *obj) {
	SpeedMessage1 *msg1 = (SpeedMessage1 *)obj;

	check_part_protobuf(msg1);

	check_val(msg1->field1() == "");
	check_val(msg1->field2() == 8);
	check_val(msg1->field3() == 2066379);
	check_val(msg1->field4() == "3K+6)#");
	check_val(msg1->field9() == "10)2uiSuoXL1^)v}icF@>P(j<t#~tz\\lg??S&(<hr7EVs\'l{\'5`Gohc_(=t eS s{_I?iCwaG]L\'*Pu5(&w_:4{~Z");
	check_val(msg1->field12() == true);
	check_val(msg1->field13() == false);
	check_val(msg1->field14() == true);

	check_val(msg1->field17() == false);
	check_val(msg1->field18() == "{=Qwfe~#n{");
	check_val(msg1->field67() == 1591432);
	check_val(msg1->field100() == 31);

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

static size_t encode_protobuf(void *state, void *obj, char *buf, size_t buflen) {
	SpeedMessage1 *msg1 = (SpeedMessage1 *)obj;
	int len;
	uint8_t *p;
	uint8_t *p_end;
	(void)state;

	len = msg1->ByteSize();
	if(len > (int)buflen) return 0;
	p = (uint8_t *)buf;
	p_end = msg1->SerializeWithCachedSizesToArray(p);
	if((p_end - p) < len) {
		printf("Encoder failed: encode len = %d, obj len = %d\n", (int)(p_end - p), len);
		return 0;
	}
	return len;
}

static void * decode_protobuf(void *state, void *obj, char *buf, size_t len) {
	SpeedMessage1 *msg1 = (obj) ? (SpeedMessage1 *)obj : new SpeedMessage1();
	(void)state;

	if(!msg1->ParseFromArray(buf, len)) {
		printf("Decoder failed: \n");
		free_protobuf(msg1);
		msg1 = NULL;
	}
	return msg1;
}

BENCH_ENC_REG(protobuf, speed1, "Benchmark: protobuf system.")

}// end: extern "C"

