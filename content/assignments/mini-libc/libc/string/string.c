/* SPDX-License-Identifier: BSD-3-Clause */

#include <string.h>

char *strcpy(char *destination, const char *source)
{
	char *src_copy = source;
	char *dst_copy = destination;
	
	/* Copy byte by byte from source to destination */
	while (*src_copy != '\0') {
		*dst_copy = *src_copy;
		src_copy++;
		dst_copy++;
	}

	*dst_copy = '\0';
	return destination;
}


char *strncpy(char *destination, const char *source, size_t len)
{
	const char *src_copy = source;
	char *dst_copy = destination;
	int it = 0;

	/* Copy at most n bytes from source to destination */
	while (*src_copy != '\0' && it < len) {
		*dst_copy = *src_copy;
		src_copy++;
		dst_copy++;

		it++;
	}
	while (it < len) {
		*dst_copy = '\0';
		it++;
	}

	return destination;
}


char *strcat(char *destination, const char *source)
{
	char *dst_copy = destination;
	const char *src_copy = source;

	/* Reach the end of destination string */
	while (*dst_copy != '\0') {
		dst_copy++;
	}

	/* Strcpy functionality as we start to copy chars from src_copy to dst_copy */
	while (*src_copy != '\0') {
		*dst_copy = *src_copy;
		src_copy++;
		dst_copy++;
	}
	*dst_copy = '\0';

	return destination;
}


char *strncat(char *destination, const char *source, size_t len)
{
	int it = 0;
	char *dst_copy = destination;

	/* Reach the end of destination string */
	while (*dst_copy != '\0') {
		dst_copy++;
	}

	/* Copy byte by byte from source to destination */
	while (*source != '\0' && it < len) {
		*dst_copy = *source;
		source++;
		dst_copy++;

		it++;
	}

	/* String terminator */
	*dst_copy = '\0';
	return destination;
}


int strcmp(const char *str1, const char *str2)
{
	/* Compare byte with byte */
	while (*str1 != '\0' && *str2 != '\0') {

		if (*str1 < *str2) {
			return -1;

		} else if (*str1 > *str2) {
			return 1;

		}
		str1++;
		str2++;
	}

	/* Check which one reached the end */
	if (*str1 == '\0' && *str2 == '\0') {
		return 0;

	} else if (*str1 == '\0') {
		return -1;

	}
	return 1;
}


int strncmp(const char *str1, const char *str2, size_t len)
{
	int it = 0;
	while (*str1 != '\0' && *str2 != '\0' && it < len) {
		if (*str1 < *str2) {
			return -1;

		} else if (*str1 > *str2) {
			return 1;
		}
		str1++;
		str2++;
		it++;
	}

	return 0;
}


size_t strlen(const char *str)
{
	size_t i = 0;

	for (; *str != '\0'; str++, i++)
		;

	return i;
}


char *strchr(const char *str, int c)
{
	/* Search for the first occurance of c in str */
	while (*str != '\0') {
		if (*str == c) {
			return str;
		}
		str++;
	}

	return NULL;
}


char *strrchr(const char *str, int c)
{
	/* Search for the last occurance of c in str */
	const char *copy_str = NULL;
	while (*str != '\0') {
		if (*str == c) {
			copy_str = str;
		}
		str++;
	}

	return copy_str;
}


char *strstr(const char *str1, const char *str2)
{
	const char *copy_str1 = NULL;
	const char *copy_str2 = NULL;

	while (*str1 != '\0') {

		/* Found the first char */
		if (*str1 == *str2) {
			copy_str1 = str1;
			copy_str2 = str2;

			int is_substring = 1;
			/* Check the entire substring */
			while (*copy_str2 != '\0'&& *copy_str1 != '\0') {
				if (*copy_str1 != *copy_str2) {
					is_substring = 0;
					break;
				}
				copy_str1++;
				copy_str2++;
			}

			/* Return the start pointer of the substring */
			if (is_substring == 1) {
				return str1;
			}
		}
		str1++;
	}

	return NULL;
}


char *strrstr(const char *str1, const char *str2)
{
	const char *last_occurance = NULL;
	const char *copy_str1 = NULL;
	const char *copy_str2 = NULL;

	while (*str1 != '\0') {

		/* Found the first char */
		if (*str1 == *str2) {
			copy_str1 = str1;
			copy_str2 = str2;

			/* Check the entire substring */
			int is_substring = 1;
			while (*copy_str2 != '\0') {

                /* Finish before the seqeunce is found */
                if (*copy_str1 == '\0') {
                    is_substring = 0;
                    break;
                }

                /* Bytes are differnt */
				if (*copy_str1 != *copy_str2) {
					is_substring = 0;
					break;
				}
				copy_str1++;
				copy_str2++;
			}

			/* The last occurance found */
			if (is_substring == 1) {
				last_occurance = str1;
			}
		}
		str1++;
	}

	return last_occurance;
}


void *memcpy(void *destination, const void *source, size_t num)
{
	/* Copy byte by byte from source to destination */
	int it = 0;
    while (*(char *)source != '\0' && it < num) {
        *(char *)destination = *(char *)source;

        source++;
        destination++;
		it++;
    }

	return destination;
}


void *memmove(void *destination, const void *source, size_t num)
{
	/* Same functionality as memcpy, but using an intermmediate buffer */
	char buffer[num];
	memcpy(buffer, (char *)source, num);
	memcpy((char *)destination, buffer, num);
	return destination;
}


int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
	/* Same functionality as strncmp, but the parametrs are void pointers */
	return strncmp((char *)ptr1, (char *)ptr2, num);
}


void *memset(void *source, int value, size_t num)
{
	/* Iterate through source and change byte with byte */
	int it = 0;
	while ((char *)source != NULL && it < num) {
		*(char *)source = (unsigned char) value;

		source++;
		it++;
	}

	return source;
}
