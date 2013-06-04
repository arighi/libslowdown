/*
 *  libslowdown - slow down open() syscalls
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Copyright (C) 2013, Andrea Righi <andrea@betterlinux.com>
 */

#define _GNU_SOURCE
#define __USE_GNU

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef DEBUG
#define DPRINTF(format, args...) {				\
		fprintf(stderr, "debug: " format, ##args);	\
		fflush(stderr);					\
	}
#else
#define DPRINTF(format, args...)
#endif

#define DEFAULT_SLEEP_MS	25

static int sleep_us = DEFAULT_SLEEP_MS * 1000;
static char *prefix = "/";

int open(const char *, int, ...) __attribute__ ((weak, alias("wrap_open")));
int open64(const char *, int, ...) __attribute__ ((weak, alias("wrap_open64")));

static int (*orig_open)(const char *, int, ...) = NULL;
static int (*orig_open64)(const char *, int, ...) = NULL;

static int __do_wrap_open(const char *name, int flags, mode_t mode,
			  int (*func_open)(const char *, int, ...))
{
	char abs_name[PATH_MAX] = {};

	if (realpath(name, abs_name)) {
		if (!strncmp(abs_name, prefix, strlen(prefix))) {
			DPRINTF("slowing down %d ms open(%s, 0x%x, 0x%x)\n",
				 sleep_us / 1000, abs_name, flags, mode);
			usleep(sleep_us);
		}
	}
	return func_open(name, flags, mode);
}

int wrap_open(const char *name, int flags, ...)
{
	va_list args;
	mode_t mode;

	va_start(args, flags);
	mode = va_arg(args, mode_t);
	va_end(args);

	return __do_wrap_open(name, flags, mode, orig_open);
}

int wrap_open64(const char *name, int flags, ...)
{
	va_list args;
	mode_t mode;

	va_start(args, flags);
	mode = va_arg(args, mode_t);
	va_end(args);

	return __do_wrap_open(name, flags, mode, orig_open64);
}

void _init(void)
{
	char *sleep_var = getenv("SLOWDOWN_MS");
	char *slowdown_prefix = getenv("SLOWDOWN_PREFIX");

	if (sleep_var)
		sleep_us = atoi(sleep_var) * 1000;
	if (slowdown_prefix)
		prefix = slowdown_prefix;

        orig_open = dlsym(RTLD_NEXT, "open");
	if (!orig_open) {
		fprintf(stderr, "error: missing symbol open!\n");
		exit(1);
	}
        orig_open64 = dlsym(RTLD_NEXT, "open64");
	if (!orig_open64) {
		fprintf(stderr, "error: missing symbol open64!\n");
		exit(1);
	}
}
