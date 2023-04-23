// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <stdarg.h>
#include <errno.h>

int close(int fd)
{
	int ok = syscall(__NR_close, fd);;
	if (ok < 0) {
		errno = -ok;
		return -1;
	}
	return ok;
}
