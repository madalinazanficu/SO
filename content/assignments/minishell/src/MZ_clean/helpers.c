#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "utils.h"

#define READ		0
#define WRITE		1

int redirect_out(simple_command_t *s, int flags)
{
	char *out = get_word(s->out);
	int status = 0;

	// Redirection only for output
	if (out != NULL) {
		int fd = open(out, flags, 0644);
		if (fd < 0) {
			status = 1;
		}
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}

	free(out);
	return status;
}

int redirect_err(simple_command_t *s, int flags)
{
	char *err = get_word(s->err);
	int status = 0;

	// Redirection only for error
	if (err != NULL) {
		int fd = open(err, flags, 0644);
		if (fd < 0) {
			status = 1;
		}
		dup2(fd, STDERR_FILENO);
		close(fd);
	}

	free(err);
	return status;
}

int redirect_in(simple_command_t *s)
{
	char *in = get_word(s->in);
	int status = 0;

	// Input redirection
	if (in != NULL) {
		int fd = open(in, O_RDONLY, 0644);
		if (fd < 0) {
			status = 1;
		}
		dup2(fd, STDIN_FILENO);
		close(fd);
	}
	free(in);
	return status;
}

int redirects(simple_command_t *s)
{

	int status = 0;

	// Extract file names and flags from the command
	char *out = get_word(s->out);
	char *err = get_word(s->err);
	int io_flags = s->io_flags;
	int flags = 0;
	int out1, out2;

	// Create the flags based on APPEND or TRUNC mode
	if (io_flags == 2 || io_flags == 1) {
		flags = O_WRONLY | O_CREAT | O_APPEND;
	} else {
		flags = O_WRONLY | O_CREAT | O_TRUNC;
	}

	// Input redirection
	redirect_in(s);

	// Output and error redirection
	if (out != NULL && err != NULL) {


		// SDOUT and STDERR are redirected to the same file
		if (strcmp(out, err) == 0) {
			int fd = open(out, flags, 0644);
			if (fd < 0) {
				status = 1;
			}

			// One file descriptor for both STDOUT and STDERR
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			close(fd);
			return status;

		} else {

			// SDOUT and STDERR are redirected to different files
			out1 = redirect_err(s, flags);
			out2 = redirect_out(s, flags);
			return out1 || out2;
		}
	}

	// Redirection only for output
	redirect_out(s, flags);

	// Redirection only for error
	redirect_err(s, flags);

	// Free the allocated resources
	free(out);
	free(err);

	return status;
}

int redirect_output_cd(simple_command_t *s)
{
	int status = 0;

	// Extract the file name for output redirection
	char *out = get_word(s->out);
	if (out != NULL) {

		int fd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0) {
			status = SHELL_EXIT;
		}
		close(fd);
	}

	// Extract the file name for error redirection
	char *err = get_word(s->err);
	if (err != NULL) {

		int fd = open(err, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0) {
			status = SHELL_EXIT;
		}
		close(fd);
	}

	// Free the allocated resources
	free(out);
	free(err);
	return status;
}
