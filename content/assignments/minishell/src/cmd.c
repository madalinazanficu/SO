// SPDX-License-Identifier: BSD-3-Clause

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

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	// The current dir needs to be expanded in order to navigate to it
	char *expanded_path = get_word(dir);
	int value = chdir(expanded_path);
	free(expanded_path);

	// Failed to change directory, exit code is 1
	if (value == -1) {
		return 1;
	}

	// Success code is 0
	return 0;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{
	return SHELL_EXIT;
}


int redirections(simple_command_t *s) {

	int shell_status = 0;

	// Extract file names and flags from the command
	char *in = get_word(s->in);
	char *out = get_word(s->out);
	char *err = get_word(s->err);
	int io_flags = s->io_flags;
	int flags = 0;

	// Create the flags based on APPEND or TRUNC mode
	if (io_flags == 2 || io_flags == 1) {
		flags = O_WRONLY | O_CREAT | O_APPEND;
	} else {
		flags = O_WRONLY | O_CREAT | O_TRUNC;
	}

	// Input redirection
	if (in != NULL) {
		int fd = open(in, O_RDONLY, 0644);
		if (fd < 0) {
			shell_status = SHELL_EXIT;
		}
		dup2(fd, STDIN_FILENO);
		close(fd);
	}

	// Both output and error redirection
	if (out != NULL && err != NULL) {
		int fd = open(out, flags, 0644);
		if (fd < 0) {
			shell_status = SHELL_EXIT;
		}

		// The file descriptor is duplicated for both STDOUT and STDERR
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		close(fd);
		return shell_status;
	}


	// Redirection only for output
	if (out != NULL) {
		int fd = open(out, flags, 0644);
		if (fd < 0) {
			shell_status = SHELL_EXIT;
		}
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}

	// Redirection only for error
	if (err != NULL) {
		int fd = open(err, flags, 0644);
		if (fd < 0) {
			shell_status = SHELL_EXIT;
		}
		dup2(fd, STDERR_FILENO);
		close(fd);
	}
	free(in);
	free(out);
	free(err);

	return shell_status;
}

int redirect_output(simple_command_t *s) {
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


/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	int status, shell_status = 0;

	/* TODO: Sanity checks. */
	if (s == NULL) {
		return SHELL_EXIT;
	}

	/* If builtin command, execute the command. */
	char *verb = get_word(s->verb);
	if (strcmp(verb, "cd") == 0) {
		free(verb);

		// Redirect the output before executing the command
		redirect_output(s);

		// Execute the command
		int cd_result = shell_cd(s->params);

		return cd_result;

	} else if (strcmp(verb, "exit") == 0 || strcmp(verb, "quit") == 0) {
		free(verb);
		return shell_exit();
	}

	/* TODO: If variable assignment, execute the assignment and return
	 * the exit status.
	 */


	/* If external command:
	 *   1. Fork new process
	 *     2c. Perform redirections in child
	 *     3c. Load executable in child
	 *   2. Wait for child
	 *   3. Return exit status
	 */
	pid_t ret_pid, pid;
	
	
	// Extract parameters for child process
	int args_size = 0;
	char **args = get_argv(s, &args_size);


	pid = fork();
	switch (pid) {

		// Error
		case -1:
			shell_status = -1;
			break;
		
		// Child process
		case 0:
			redirections(s);
			shell_status = execvp(verb, args);
			exit(shell_status);

		// Parent process
		default:
			ret_pid = waitpid(pid, &status, 0);
			if (ret_pid < 0) {
				shell_status = -1;
			}
			if (WIFEXITED(status)) {
				shell_status = WEXITSTATUS(status);
			}
			break;
	}

	// Free the allocated resources
	for (int i = 0; i < args_size; i++) {
		free(args[i]);
	}

	free(args);
	free(verb);

	return shell_status;
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool run_in_parallel(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* TODO: Execute cmd1 and cmd2 simultaneously. */

	return true; /* TODO: Replace with actual exit status. */
}

/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2).
 */
static bool run_on_pipe(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	/* TODO: Redirect the output of cmd1 to the input of cmd2. */

	return true; /* TODO: Replace with actual exit status. */
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	/* TODO: sanity checks */

	if (c->op == OP_NONE) {
		/* TODO: Execute a simple command. */
		return parse_simple(c->scmd, level, father);
	}

	int out1 = 0;
	int out2 = 0;
	int status = 0;

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* TODO: Execute the commands one after the other. */
		parse_command(c->cmd1, level + 1, c);
		parse_command(c->cmd2, level + 1, c);

		break;

	case OP_PARALLEL:
		/* TODO: Execute the commands simultaneously. */
		break;


	// This is: cmd1 || cmd2 -> ma opresc la primul success, return code = 0
	case OP_CONDITIONAL_NZERO:
		/* TODO: Execute the second command only if the first one
		 * returns non zero.
		 */

		out1 = parse_command(c->cmd1, level + 1, c);
		if (out1 == 0) {
			return 0;
		}
		out2 = parse_command(c->cmd2, level + 1, c);
		status = out2;
		break;


	// This is: cmd1 && cmd2 -> ma opresc la primul fail, return code == 1
	case OP_CONDITIONAL_ZERO:
		/* TODO: Execute the second command only if the first one
		 * returns zero.
		 */

		out1 = parse_command(c->cmd1, level + 1, c);
		if (out1 != 0) {
			return 1;
		}
		out2 = parse_command(c->cmd2, level + 1, c);
		status = out2;
		break;


	case OP_PIPE:
		/* TODO: Redirect the output of the first command to the
		 * input of the second.
		 */
		break;

	default:
		return SHELL_EXIT;
	}

	return status; /* TODO: Replace with actual exit code of command. */
}
