// SPDX-License-Identifier: BSD-3-Clause

#include "functions.h"

// Finds the padding value in order to align
// either the metadata or the payload to 8 bytes
size_t padded(size_t size)
{
	size_t padding = 0;

	if (size % 8 == 0)
		padding = 0;
	else
		padding = 8 - (size % 8);

	return size + padding;
}

// Iterate through the list of blocks and merge adjacent free blocks
void coalesce_blocks(struct block_meta *list_head,
						struct block_meta **list_tail)
{
	DIE(list_head == NULL, "list_head is NULL");

	struct block_meta *block1 = list_head;
	struct block_meta *block2 = NULL;

	if (block1->next != NULL)
		block2 = list_head->next;

	while (block2 != NULL) {
		if (block1->status == STATUS_FREE && block2->status == STATUS_FREE) {
			block1->size = padded(block1->size)
						+ padded(block2->size) + META_DATA_PADDED;

			// Update the list tail if necessary
			if (block2 == *list_tail)
				*list_tail = block1;

			block1->next = block2->next;
			block2 = block2->next;

		} else {
			block1 = block1->next;
			block2 = block2->next;
		}
	}
}

// Splits the block in two blocks
void split_block(struct block_meta *block, size_t split_size,
					struct block_meta **list_tail)
{
	size_t total_block_size = padded(block->size);

	struct block_meta *block1 = block;

	block1->size = split_size;
	block1->status = STATUS_ALLOC;

	struct block_meta *block2 = (struct block_meta *) ((void *)block1
													+ padded(block1->size)
													+ META_DATA_PADDED);

	block2->size = total_block_size - padded(block1->size) - META_DATA_PADDED;
	block2->status = STATUS_FREE;

	block2->next = block->next;
	block1->next = block2;

	if (block1 == *list_tail)
		*list_tail = block2;
}


// Coalesces the free blocks and then iterates through the list of blocks
// to find a free block with size greater than @param size
// Corner case: the split of the block is done only if the remaining block
// is greater than the minimum payload and the size of the metadata
void *find_free_best(size_t size, struct block_meta *list_head,
						struct block_meta **list_tail)
{
	// Merge all adjacent free blocks before searching for a free block
	coalesce_blocks(list_head, list_tail);

	struct block_meta *block = list_head;
	struct block_meta *best = NULL;
	size_t best_size = 0;

	while (block != NULL) {
		if (block->status == STATUS_FREE && block->size >= size) {

			// Find the best block
			if (best == NULL || best_size > block->size - size) {
				best = block;
				best_size = block->size - size;
			}
		}
		block = block->next;
	}

	if (best != NULL) {
		// Split the block if the remaining block is big enough
		if (best->size - size >= MIN_BLOCK_SIZE)
			split_block(best, size, list_tail);

		best->status = STATUS_ALLOC;
		return best;
	}

	return NULL;
}

// Used for expanding the block to the requested size
// in case the reallocation is performed using a bigger size
// Find an adjacent free block and merge them
size_t expand_block(struct block_meta *block, size_t size,
					struct block_meta **list_tail)
{
	DIE(block == NULL, "expand block: block is NULL");

	struct block_meta *next_block = block->next;

	while (next_block != NULL) {
		if (next_block->status == STATUS_FREE) {
			size_t expanded_size = padded(block->size)
								+ padded(next_block->size)
								+ META_DATA_PADDED;

			block->size = expanded_size;

			// Update the list tail if needed
			if (next_block == *list_tail)
				*list_tail = block;

			block->next = next_block->next;
			next_block = next_block->next;

			// Check if the block is big enough => no need to expand more
			if (block->size >= padded(size)) {

				// In case the block is too big, split it
				if (block->size > padded(size))
					if (block->size >= MIN_BLOCK_SIZE + size)
						split_block(block, size, list_tail);

				return 1;
			}

		} else {
			return 0;
		}
	}
	return 0;
}

// Based on this size and the size of the last block, the remaining size
// is calculated and the last block is expanded
// @param is_realloc = 1 if the function is called from realloc, 0 otherwise
// For realloc, the block we are trying to reallocate is ALLOCATED
void *use_last_block(size_t size, size_t is_realloc,
						struct block_meta *list_tail)
{
	// If the last block is free, try to expand it
	if (list_tail->status == STATUS_FREE || is_realloc == 1) {
		size_t intermmidate_size = padded(size) - padded(list_tail->size);
		struct block_meta *block = sbrk(intermmidate_size);

		DIE(block == (void *) -1, "sbrk failed");
		list_tail->size = size;
		list_tail->status = STATUS_ALLOC;
		return (void *)(list_tail + 1);
	}

	return NULL;
}
