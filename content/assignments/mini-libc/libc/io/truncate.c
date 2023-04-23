// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

int truncate(const char *path, off_t length)
{
	int ok = syscall(__NR_truncate, path, length);
	if (ok < 0) {
		errno = -ok;
		return -1;
	}
	return ok;
}
