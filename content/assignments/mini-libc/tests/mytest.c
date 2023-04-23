#include <fcntl.h>
#include <internal/syscall.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>

//#define NON_EXISTENT_FILE "./file_NONEXISTENT"



static int test_open(void)
{
	int fd;

	fd = open("MADA.txt", O_RDONLY);

	return fd == -1 && errno == ENOENT;
}


int main(void)
{
    int ret = test_open();
    char str[1];
    str[0] =  ret + '0';
    str[1] = '\0';


    puts(str);
	
    return 0;
}