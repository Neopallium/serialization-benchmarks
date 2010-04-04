/***************************************************************************
 * Copyright (C) 2007-2010 by Robert G. Jakabosky <bobby@neoawareness.com> *
 *                                                                         *
 ***************************************************************************/

#include <google/protobuf/stubs/common.h>

extern "C" {
#include "protobuf.h"
}

void init_protobuf() {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
}

void cleanup_protobuf() {
#ifdef BENCH_PROTOBUF
	google::protobuf::ShutdownProtobufLibrary();
#endif
}

