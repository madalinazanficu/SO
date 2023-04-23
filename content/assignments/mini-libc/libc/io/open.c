// SPDX-License-Identifier: BSD-3-Clause

#include <fcntl.h>
#include <internal/syscall.h>
#include <stdarg.h>
#include <errno.h>

int open(const char *filename, int flags, ...)
{	
	/* Extract open mode from ... */
	va_list args;
	va_start(args, flags);
	mode_t mode = va_arg(args, mode_t);
	va_end(args);

	int fd = syscall(__NR_open, filename, flags, mode);

	if (fd < 0) {
		errno = -fd;
		return -1;
	}

	return fd;
}
