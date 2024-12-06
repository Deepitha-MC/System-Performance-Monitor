/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * system.c
 */

#define _GNU_SOURCE

#include <sys/time.h>
#include <unistd.h>
#include "system.h"

/**
 * Needs:
 *   gettimeofday()
 *   nanosleep()
 *   unlink()
 *   vsnprintf()
 *   sysconf()
 */

uint64_t
ref_time(void)
{
	struct timeval timeval;

	if (gettimeofday(&timeval, 0)) {
		TRACE("gettimeofday()");
		return 0;
	}
	return (uint64_t)timeval.tv_sec * 1000000 + (uint64_t)timeval.tv_usec;
}

void
us_sleep(uint64_t us)
{
	struct timespec in, out;

	in.tv_sec = (time_t)(us / 1000000);
	in.tv_nsec = (long)(us % 1000000) * 1000;
	while (nanosleep(&in, &out)) {
		in = out;
	}
}

void
file_delete(const char *pathname)
{
	if (safe_strlen(pathname)) {
		if (unlink(pathname)) {
			/* ignore */
		}
	}
}

void
safe_sprintf(char *buf, size_t len, const char *format, ...)
{
	va_list ap;

	assert( (!len || buf) && format );

	va_start(ap, format);
	if ((int)len <= vsnprintf(buf, len, format, ap)) {
		va_end(ap);
		EXIT("software");
	}
	va_end(ap);
}

size_t
safe_strlen(const char *s)
{
	return s ? strlen(s) : 0;
}

size_t
page_size(void)
{
	long size;

	if ((0 >= (size = sysconf(_SC_PAGESIZE)))) {
		EXIT("sysconf()");
		return 0;
	}
	return (size_t)size;
}

void *
memory_align(void *p, size_t n)
{
	size_t r;

	if ((r = (size_t)p % n)) {
		r = n - r;
	}
	return (void *)((char *)p + r);
}
