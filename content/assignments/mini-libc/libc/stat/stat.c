// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <internal/syscall.h>

int stat(const char *restrict path, struct stat *restrict buf)
{
	int ok = syscall(__NR_stat, path, buf);

	/* Invalid path */
	if (ok < 0) {
		errno = -ok;
		return -1;
	}

	return ok;
}
