/***************************************************************************
 * Copyright (C) 2007-2010 by Robert G. Jakabosky <bobby@neoawareness.com> *
 *                                                                         *
 ***************************************************************************/
#ifndef __BTIMER_H__
#define __BTIMER_H__

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
  if(clock_gettime(CLOCK_MONOTONIC,&tp) != 0) {
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

#endif /* __BTIMER_H__ */
