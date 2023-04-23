
#pragma once

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "printf.h"
#include "helpers.h"

#define THRESHOLD_MMAP 1024 * 128
#define HEAP_PREALLOC 1024 * 128
#define PAGE_SIZE 4096

#define META_DATA_PADDED padded(sizeof(struct block_meta))
#define MIN_PAYLOAD padded(1)
#define MIN_BLOCK_SIZE (META_DATA_PADDED + MIN_PAYLOAD)

size_t padded(size_t size);

void coalesce_blocks(struct block_meta *list_head,
                        struct block_meta **list_tail);

void split_block(struct block_meta *block, size_t split_size,
                    struct block_meta **list_tail);

void *find_free_best(size_t size, struct block_meta *list_head,
						struct block_meta **list_tail);

size_t expand_block(struct block_meta *block, size_t size,
					struct block_meta **list_tail);

void *use_last_block(size_t size, size_t is_realloc,
                        struct block_meta *list_tail);