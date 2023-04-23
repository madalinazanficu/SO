// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

off_t lseek(int fd, off_t offset, int whence)
{
	off_t off = 0;

	/* Bad file number */
	if (fd < 0) {
		errno = EBADF;
		return -1;
	}

	/* Invalid offset */
	if (offset < 0) {
		errno = EINVAL;
		return -1;
	}
	/* whence == SEEK_SET => Set the the file cursor = offset
	   whence == SEEK_CUR => Set the file cursor = cursor + offset
	   whence == SEEK_END => Set the file cursor = file_size + offset */

	off = syscall(__NR_lseek, fd, offset, whence);

	if (off < 0) {
		errno = EINVAL;
		return -1;
	}

	return off;
}
