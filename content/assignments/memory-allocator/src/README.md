### Zanficu Madalina 333CA
## Memory allocator

# Description 
The implementation includes malloc, calloc, free, and realloc functionalities.
Helpers functions are implemented in functions.c

# Implementation and design choices
For the blocks managment, I have decided to store them in the following way:
- I have a global variable, which is a pointer to the first block of the heap.
- Each block has a header and a payload. The header contains:
  - the size of the block
  - a pointer to the next block
  - a flag which indicates if the block is free or not

- I have decided to use this list to store only the blocks allocated on the 
heap (allocated using sbrk), because I am not using the blocks allocated 
with mmap for any of the operations such as: split_blcok, merge_blocks, etc.

- Moreover, I am using a global variable to store the last block allocated 
on the heap. This is done in order to avoid traversing the list of blocks 
each time I need to allocate a new block.


1. Malloc function cases are presenting in the comments of the function,
and calloc function is similar to malloc, but it also initializes the memory
with 0.

2. Free function - for the blocks allocated with mmap, I am using 
the munmap function to free the memory. For the blocks allocated with sbrk,
I only set them as free.

3. Realloc function - I am using the following cases:
  - if the new size is smaller than the old size, I am splitting the block
  - if the new size is bigger than the old size, I am checking if the 
    next block is free and if it is big enough to store the new size. If it is,
    I am merging the blocks  and I am splitting the new block if it is bigger
    than the new size.
  - if the last block is free and it is big enough to store the new size,
    I am expanding it using the use_last_block function.
  - otherwise, I am allocating a new block and I am copying the old data
    in the new block.

Remarkable functions:
1. request_space - this function is used to allocate a new block either 
with mmap or sbrk. The main idea is to check if the size of the block is 
bigger than the threshold. The threshold could be the THRESHOLD_MMAP or
PAGE_SIZE, depending on the case (malloc / calloc).

2. find_free_best - the main idea is to find the best free block to reuse.
The best block is the one which has the proper size and is the closest to
the size of the block I want to allocate.

3. use_last_block - this function is used to expand the last block on the heap.
Used for malloc, calloc and realloc.


# Homework opinions
I have tried my best to have a proper coding style, since the Docker image
was not working for me. I have tried to fix my code taking into account
the comments from moodle. 

Overall: I have learned a lot about memory management, I never thought
malloc / calloc are so complex. 