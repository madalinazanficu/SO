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

extern char **environ;

word_t *path = NULL;
char *cwd = NULL;

static char *pwd()
{
	// Allocate memory only once then reuse it
	if (cwd == NULL) {
		cwd = calloc(1024, sizeof(char));
	}

	// Get the current working directory using getcwd()
	getcwd(cwd, 1024);


	// Print the result
	printf("%s\n", cwd);

	return cwd;
}

/**
 * Internal change-directory command.
 */
static bool shell_cd(word_t *dir)
{
	// First call -> allocate memory for the global path
	if (path == NULL) {
		path = calloc(1, sizeof(word_t));
	}

	// The current dir needs to be expanded in order to navigate to it
	char *expanded_path = get_word(dir);
	int value = chdir(expanded_path);
	free(expanded_path);


	// Failed to change directory
	if (value == -1) {
		return false;
	}

	// Success
	path = dir;
	return true;
}

/**
 * Internal exit/quit command.
 */
static int shell_exit(void)
{
	free(path);
	free(cwd);
	//exit(EXIT_SUCCESS);

	//printf("Exit Exit\n");
	//return EXIT_SUCCESS; /* TODO: Replace with actual exit code. */
	return SHELL_EXIT;
}

/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	/* TODO: Sanity checks. */
	if (s == NULL) {
		return SHELL_EXIT;
	}

	
	/* If builtin command, execute the command. */
	char *verb = get_word(s->verb);
	if (strcmp(verb, "cd") == 0) {
		free(verb);
		shell_cd(s->params);
		return EXIT_SUCCESS;

	} else if (strcmp(verb, "exit") == 0 || strcmp(verb, "quit") == 0) {
		free(verb);
		//printf("Exit\n");
		return shell_exit();

	} else if (strcmp(verb, "pwd") == 0) {
		free(verb);
		pwd();
		return EXIT_SUCCESS;
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
	int status, shell_status = 0;
	
	
	// Extract parameters for child process
	int args_size = 0;
	char **args = get_argv(s, &args_size);


	pid = fork();
	switch (pid) {

		// Error
		case -1:
			shell_status = SHELL_EXIT;
			break;
		
		// Child process
		case 0:
			//printf("Child process\n");
			execvp(verb, args);
			exit(EXIT_SUCCESS);
			//printf("Child process\n");


		// Parent process
		default:
			ret_pid = waitpid(pid, &status, 0);
			if (ret_pid < 0) {
				shell_status = SHELL_EXIT;
			}
			break;
	}

	// Free the allocated resources
	for (int i = 0; i < args_size; i++) {
		free(args[i]);
	}

	free(args);
	free(verb);

	return shell_status; /* TODO: Replace with actual exit status. */
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
		// return 0; /* TODO: Replace with actual exit code of command. */
	}

	switch (c->op) {
	case OP_SEQUENTIAL:
		/* TODO: Execute the commands one after the other. */
		break;

	case OP_PARALLEL:
		/* TODO: Execute the commands simultaneously. */
		break;

	case OP_CONDITIONAL_NZERO:
		/* TODO: Execute the second command only if the first one
		 * returns non zero.
		 */
		break;

	case OP_CONDITIONAL_ZERO:
		/* TODO: Execute the second command only if the first one
		 * returns zero.
		 */
		break;

	case OP_PIPE:
		/* TODO: Redirect the output of the first command to the
		 * input of the second.
		 */
		break;

	default:
		return SHELL_EXIT;
	}

	return 0; /* TODO: Replace with actual exit code of command. */
}
