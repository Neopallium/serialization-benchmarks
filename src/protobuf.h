/***************************************************************************
 * Copyright (C) 2007-2010 by Robert G. Jakabosky <bobby@neoawareness.com> *
 *                                                                         *
 ***************************************************************************/
#ifndef __PROTOBUF_H__
#define __PROTOBUF_H__

#ifdef BENCH_PROTOBUF
void init_protobuf();
void cleanup_protobuf();
#else
#define init_protobuf()
#define cleanup_protobuf()
#endif

#endif /* __PROTOBUF_H__ */
