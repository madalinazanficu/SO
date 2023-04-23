// SPDX-License-Identifier: BSD-3-Clause

#include "osmem.h"
#include "functions.h"

struct block_meta *list_head;
struct block_meta *list_tail;


// Analize the size of the request and the moment of allocation and decides:
// - if the request is a heap preallocation or a normal allocation
// - if the block is allocated with mmap or sbrk
void *request_space(size_t size, size_t is_heap_prealloc, size_t threshold)
{
	// Ensure that the block is aligned to 8 bytes
	size_t payload_padded = padded(size);
	size_t total_size = META_DATA_PADDED + payload_padded;

	if (is_heap_prealloc)
		total_size = size;

	// Allocate the memory based on the size of the request
	void *request;
	struct block_meta *block;

	if (total_size > threshold) {
		request = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE
												   | MAP_ANONYMOUS, -1, 0);
		DIE(request == MAP_FAILED, "mmap failed");

		block = request;
		block->status = STATUS_MAPPED;

	} else {
		request = sbrk(total_size);
		DIE(request == (void *) -1, "sbrk failed");

		block = request;
		block->status = STATUS_ALLOC;
	}

	// Init metadata for the new block
	block->size = size;
	block->next = NULL;

	if (is_heap_prealloc == 1) {
		block->status = STATUS_FREE;
		block->size = size - META_DATA_PADDED;
	}
	return block;
}


// The main logic of the allocator:
// Cases:
// 1. Heap preallocation - the first allocation is done with sbrk
// 2. Allocation which requires mmap => don't add the block to the list
// 3. Find a free block of sufficient size
// 4. Expand block: the last block is free, but it is not big enough
// 5. Normal allocation - the block is allocated with sbrk
void *os_malloc(size_t size)
{
	if (size == 0)
		return NULL;

	struct block_meta *block;

	// Heap preallocation
	if (list_head == NULL) {
		if (size < THRESHOLD_MMAP) {
			block = request_space(HEAP_PREALLOC, 1, THRESHOLD_MMAP);
			list_head = block;
			list_tail = block;
			block = find_free_best(size, list_head, &list_tail);
			return (void *)(block + 1);
		}
	}

	// Allocation which requires mmap => don't add the block to the list
	if (size >= THRESHOLD_MMAP) {
		block = request_space(size, 0, THRESHOLD_MMAP);
		return (void *)(block + 1);
	}

	// Before allocating more, try to find a free block
	block = find_free_best(size, list_head, &list_tail);
	if (block != NULL)
		return (void *)(block + 1);

	// In case the last block is free, try to expand it
	if (list_tail->status == STATUS_FREE) {
		block = use_last_block(size, 0, list_tail);
		if (block != NULL)
			return (void *)(list_tail + 1);
	}

	// No free block of sufficient size found, request more space from heap
	if (block == NULL) {
		block = request_space(size, 0, THRESHOLD_MMAP);
		DIE(block == NULL, "request_space failed");

		// Add the block to the list of heap blocks
		list_tail->next = block;
		list_tail = list_tail->next;
	}

	// Return the pointer to the payload
	return (void *)(block + 1);
}


// Free the block pointed by @param ptr
// Use munmap if the block was allocated with mmap
// In case the block was allocated with sbrk, mark it as free
void os_free(void *ptr)
{
	if (ptr == NULL)
		return;

	struct block_meta *block = ptr - META_DATA_PADDED;

	if (block->status == STATUS_MAPPED) {
		size_t padded_payload = padded(block->size);
		size_t total_size = META_DATA_PADDED + padded_payload;

		munmap(block, total_size);
		return;
	}

	block->status = STATUS_FREE;
}

void *os_calloc(size_t nmemb, size_t size)
{
	if (nmemb == 0 || size == 0)
		return NULL;

	size_t total_size = nmemb * size;

	struct block_meta *block;

	// Heap preallocation
	if (list_head == NULL) {
		if (total_size < PAGE_SIZE - META_DATA_PADDED) {
			block = request_space(HEAP_PREALLOC, 1, THRESHOLD_MMAP);
			list_head = block;
			list_tail = block;
			block = find_free_best(total_size, list_head, &list_tail);
			return memset((void *)(block + 1), 0, padded(total_size));
		}
	}

	// Allocation which requires mmap => don't add the block to the list
	if (total_size >= PAGE_SIZE - META_DATA_PADDED) {
		block = request_space(total_size, 0, PAGE_SIZE);
		return memset((void *)(block + 1), 0, padded(total_size));
	}

	// Find a free block of sufficient size
	block = find_free_best(total_size, list_head, &list_tail);
	if (block != NULL)
		return memset((void *)(block + 1), 0, padded(total_size));

	// Expand block: the last block is free, but it is not big enough
	if (list_tail->status == STATUS_FREE) {
		block = use_last_block(total_size, 0, list_tail);
		if (block != NULL)
			return memset((void *)(list_tail + 1), 0, padded(total_size));
	}

	// No free block of sufficient size found, request more space
	if (block == NULL) {
		block = request_space(total_size, 0, PAGE_SIZE);
		DIE(block == NULL, "request_space failed");

		list_tail->next = block;
		list_tail = list_tail->next;
	}

	return memset((void *)(block + 1), 0, padded(total_size));
}


// Reallocates the memory block pointed to by ptr
void *os_realloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return os_malloc(size);

	if (size == 0) {
		os_free(ptr);
		return NULL;
	}

	// Get the block meta data
	struct block_meta *block = (struct block_meta *)ptr - 1;

	if (block->status == STATUS_FREE)
		return NULL;

	// The block doesn't need resizing
	if (block->size == size)
		return ptr;

	// Used for memcpy in case of malloc
	size_t min_size = block->size < size ? block->size : size;

	// Reallocation which requires mmap
	if (block->status == STATUS_MAPPED || size > THRESHOLD_MMAP) {
		void *new_ptr = os_malloc(size);

		DIE(new_ptr == NULL, "os_malloc failed");
		memcpy(new_ptr, ptr, min_size);
		os_free(ptr);
		return new_ptr;
	}

	// Based on the new size, decide if the block should be split or expanded
	if (block->size > size) {

		// Decrease size of the block if it is too big
		if (block->size >= MIN_BLOCK_SIZE + size)
			split_block(block, size, &list_tail);

		return ptr;
	}

	// Expand the block size if possible
	size_t expanded = expand_block(block, size, &list_tail);

	if (expanded == 1)
		return ptr;


	// In case the block is the last one,
	// try to allocate enough space to get the desired size
	if (block == list_tail) {
		block = use_last_block(size, 1, list_tail);
		if (block != NULL)
			return (void *)(list_tail + 1);

		return ptr;
	}

	// The remaining case is to allocate a new block and copy the data
	void *new_ptr = os_malloc(size);

	memcpy(new_ptr, ptr, min_size);
	os_free(ptr);
	return new_ptr;
}
