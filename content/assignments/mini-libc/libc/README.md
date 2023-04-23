## Operating Systems - Mini-libc
### Zanficu Madalina 333CA

### Overview
The main purpose of this assignment was to understand how the standard C library really works:
1. Making system calls for privileged operations such as:
        - I/O functionalities: puts(), open(), close(), truncate(), lseek(), stat() 
        - memory management: malloc(), calloc(), free(), mmap(), realloc()
        - interrupt execution for a certain number of seconds: sleep(), nanosleep()

2. Understand why string operations do not use system calls.
My miniature library supports these functionalities:
- strpy, strncpy, strcat, strncat, strcmp, strncmp.
- strchr, strrchr, strstr, strrstr
- memcpy, memcmp, memset, memmove


### Implementation and resources
For the implementation I mainly used the manual pages directly from the Linux system or: 
https://man7.org/linux/man-pages/man3/string.3.html

For the syntax of the system calls, I used the read/write functions from the skeleton:
mini-libc/libc/io/read_write.c
It was a good starting point and example.

### Homework Feedback
The skeleton was very useful and helped me understand how a library is divided.
Also, the functionality provided by mem_list.c was excellent.

Translated with www.DeepL.com/Translator (free version)
