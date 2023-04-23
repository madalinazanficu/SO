// SPDX-License-Identifier: BSD-3-Clause

#include <sys/mman.h>
#include <errno.h>
#include <internal/syscall.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	long mmap_out = syscall(__NR_mmap, addr, length, prot, flags, fd, offset);
	if (mmap_out < 0) {
		errno = -mmap_out;
		return MAP_FAILED;
	}
	
	return (void *)mmap_out;
}

void *mremap(void *old_address, size_t old_size, size_t new_size, int flags)
{
	long mremap_out = syscall(__NR_mremap, old_address, old_size, new_size, flags);
	if (mremap_out < 0) {
		errno = -mremap_out;
		return MAP_FAILED;
	}
	
	return (void *)mremap_out;
}

int munmap(void *addr, size_t length)
{
	long munmap_out = syscall(__NR_munmap, addr, length);
	if (munmap_out < 0) {
		errno = -munmap_out;
		return MAP_FAILED;
	}
	
	return (void *)munmap_out;
}
