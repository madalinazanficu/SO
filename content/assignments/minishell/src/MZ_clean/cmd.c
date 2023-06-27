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
#include "helpers.h"

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


/**
 * Parse a simple command (internal, environment variable assignment,
 * external command).
 */
static int parse_simple(simple_command_t *s, int level, command_t *father)
{
	int status, shell_status = 0;

	if (s == NULL)
		return SHELL_EXIT;

	// BUILD-IN COMMANDS
	char *verb = get_word(s->verb);
	if (strcmp(verb, "cd") == 0) {
		free(verb);

		// Redirect the output before executing the command
		redirect_output_cd(s);

		// Execute the command
		return shell_cd(s->params);

	} else if (strcmp(verb, "exit") == 0 || strcmp(verb, "quit") == 0) {
		free(verb);
		return shell_exit();
	}


	// VARIABLE ASSIGNMENT, SUCH AS: var=value
	char *var = get_word(s->verb);
	if (strchr(var, '=') != NULL) {
		const char *src = s->verb->string;

		if (s->verb->next_part != NULL)
			if (s->verb->next_part->next_part != NULL) {
				char *dst = get_word(s->verb->next_part->next_part);

				// Assign the value to the variable
				setenv(src, dst, 1);
				return 0;
			}
	}
	

	// EXTERNAL COMMANDS
	int args_size = 0;
	char **args = get_argv(s, &args_size);
	pid_t ret_pid, pid;

	// Step 1: Fork new process
	pid = fork();
	switch (pid) {
		case -1:
			shell_status = -1;
			break;
	
		case 0:
			// Step 2.1: Perform redirections in child
			redirects(s);

			// Step 2.2: Load executable in child
			shell_status = execvp(verb, args);
			if (shell_status == -1)
				printf("Execution failed for '%s'\n", verb);
	
			// Step 2.3: Finish the process for the child
			exit(shell_status);

		default:
			// Step 2.4: Wait for child
			ret_pid = waitpid(pid, &status, 0);
			if (ret_pid < 0)
				shell_status = -1;
			if (WIFEXITED(status))
				shell_status = WEXITSTATUS(status);
			break;
	}

	// Free the allocated resources
	for (int i = 0; i < args_size; i++)
		free(args[i]);
	free(args);
	free(verb);

	return shell_status;
}



/**
 * Run commands by creating an anonymous pipe (cmd1 | cmd2).
 */
static bool run_on_pipe(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	int pipefd[2];
	pid_t pid1, pid2;
	int out1, out2;
	int status_child1, status_child2;
	int ret_pid2;

	// Create the pipe for the two commands
	int pipe_status = pipe(pipefd);
	if (pipe_status == -1) {
		return false;
	}

	// Create the first child process for cmd1
	pid1 = fork();
	switch (pid1) {
		case -1:
			return false;

		case 0:
			close(pipefd[READ]);
			// The output for cmd1 is written in the pipe write end
			dup2(pipefd[WRITE], STDOUT_FILENO);
			close(pipefd[WRITE]);
			out1 = parse_command(cmd1, level + 1, father);

			exit(out1);

		default:
			waitpid(pid1, &status_child1, 0);
	}

	// Create the second child process for cmd2
	pid2 = fork();
	switch (pid2) {
		case -1:
			return false;

		case 0:
			close(pipefd[WRITE]);
			// The input for cmd2 is taken from the pipe read end
			dup2(pipefd[READ], STDIN_FILENO);
			close(pipefd[READ]);
			out2 = parse_command(cmd2, level + 1, father);

			exit(out2);

		default:
			ret_pid2 = waitpid(pid2, &status_child2, 0);
			if (ret_pid2 < 0) {
				pipe_status = -1;
			}

			if (WIFEXITED(status_child2)) {
				pipe_status = WEXITSTATUS(status_child2);
			}
	}

	close(pipefd[READ]);
	close(pipefd[WRITE]);
	return pipe_status;
}

/**
 * Process two commands in parallel, by creating two children.
 */
static bool run_in_parallel(command_t *cmd1, command_t *cmd2, int level,
		command_t *father)
{
	int parallel_status = 0;
	int pid1, pid2;
	int out1, out2;

	int status_child1, status_child2;
	int ret_pid1, ret_pid2;

	// Create the first child process for cmd1
	pid1 = fork();

	switch (pid1) {
		case -1:
			return false;
		case 0:
			out1 = parse_command(cmd1, level + 1, father);
			exit(out1);
		default:
			break;
	}

	// Create the second child process for cmd2
	pid2 = fork();
	switch (pid2) {
		case -1:
			return false;

		case 0:
			out2 = parse_command(cmd2, level + 1, father);
			exit(out2);

		default:
			ret_pid2 = waitpid(pid2, &status_child2, 0);

			if (ret_pid2 < 0)
				parallel_status = -1;

			if (WIFEXITED(status_child2))
				parallel_status = WEXITSTATUS(status_child2);
			break;
	}

	ret_pid1 = waitpid(pid1, &status_child1, 0);
	if (ret_pid1 < 0)
		parallel_status = -1;

	if (WIFEXITED(status_child1))
		parallel_status = WEXITSTATUS(status_child1);

	return parallel_status; /* TODO: Replace with actual exit status. */
}

/**
 * Parse and execute a command.
 */
int parse_command(command_t *c, int level, command_t *father)
{
	
	if (c == NULL)
		return SHELL_EXIT;

	if (c->op == OP_NONE)
		return parse_simple(c->scmd, level + 1, father);

	int out1 = 0, status = 0;

	switch (c->op) {
	case OP_SEQUENTIAL:
		parse_command(c->cmd1, level + 1, c);
		parse_command(c->cmd2, level + 1, c);

		break;

	// cmd1 & cmd2 -> both cmd1 and cmd2 are executed simultaneously
	case OP_PARALLEL:
		return run_in_parallel(c->cmd1, c->cmd2, level + 1, c);

	// cmd1 || cmd2 -> stop at first success, when the return code = 0
	case OP_CONDITIONAL_NZERO:
		out1 = parse_command(c->cmd1, level + 1, c);

		if (out1 == 0) {
			return 0;
		}
		return parse_command(c->cmd2, level + 1, c);

	// cmd1 && cmd2 -> stop at first fail, when the return code != 0
	case OP_CONDITIONAL_ZERO:
		out1 = parse_command(c->cmd1, level + 1, c);

		if (out1 != 0) {
			return 1;
		}
		return parse_command(c->cmd2, level + 1, c);

	// cmd1 | cmd2 -> output of cmd1 is the input of cmd2
	case OP_PIPE:
		return run_on_pipe(c->cmd1, c->cmd2, level + 1, c);

	default:
		return SHELL_EXIT;
	}

	return status;
}
