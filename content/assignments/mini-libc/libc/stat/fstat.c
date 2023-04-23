// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <errno.h>
#include <internal/syscall.h>

int fstat(int fd, struct stat *st)
{
	int ok = syscall(__NR_fstat, fd, st);

	/* Invalid path */
	if (ok < 0) {
		errno = -ok;
		return -1;
	}

	return ok;
}
