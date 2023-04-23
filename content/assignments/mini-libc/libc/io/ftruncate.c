// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

int ftruncate(int fd, off_t length)
{
	int ok = syscall(__NR_ftruncate, fd, length);
	if (ok < 0) {
		errno = -ok;
		return -1;
	}
	return ok;
}
