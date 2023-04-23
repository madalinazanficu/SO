// SPDX-License-Identifier: BSD-3-Clause

#include <internal/mm/mem_list.h>
#include <internal/types.h>
#include <internal/essentials.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <internal/syscall.h>
#include <errno.h>

void *malloc(size_t size)
{
	long memory;

	memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory < 0) {
		errno = -memory;
		return NULL;
	}

	int ok = mem_list_add(memory, size);
	if (ok < 0) {
		return NULL;
	}

	return (void *)memory;
}

void *calloc(size_t nmemb, size_t size)
{
	long memory;

	/* Allocate nmemb * size bytes of memory */
	memory = mmap(NULL, nmemb * size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory < 0) {
		errno = -memory;
		return NULL;
	}

	/* Add the allocated memory to the list */
	int ok = mem_list_add(memory, nmemb * size);
	if (ok < 0) {
		return NULL;
	}

	/* Set the memory block on 0 */
	memset((void *)memory, 0, nmemb * size);

	return (void *)memory;
}

void free(void *ptr)
{
	/* Search the memory block in the list */
	struct mem_list *memory = mem_list_find(ptr);
	if (memory == NULL) {
		return;
	}

	/* Free the memory */
	int ok = munmap(ptr, memory->len);

	/* Remove the memory from the list */
	mem_list_del(ptr);
}

void *realloc(void *ptr, size_t size)
{
	/* According to man => the call is equivalent to malloc(size)*/
	if (ptr == NULL) {
		return malloc(size);
	}

	/* Search the memory block in the list */
	struct mem_list *memory = mem_list_find(ptr);
	if (memory == NULL) {
		return NULL;
	}

	/* New size is bigger than the old one => allocate a new memory block */
	if (size > memory->len) {
		void *new_memory = malloc(size);
		memmove(new_memory, ptr, memory->len);

		/* Free the old memory block */
		free(ptr);

		return new_memory;
	}

	/* New size is smaller than the old one */
	return ptr;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
	void *ok = realloc(ptr, nmemb * size);
	if (ok == NULL) {
		errno = -*(int *)ok;
		return NULL;
	}
	return ok;
}
