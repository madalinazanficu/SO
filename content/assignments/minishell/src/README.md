### Zanficu Madalina - Valentina
### Mini-shell assignment

## Implementation
Mini-shell core functionality stars in parse_command function. 
This function recurisvely splits the command into 2 commands taking 
into account these operators: **;**, **|**, **&**, **&&**, **||**,
until it reaches a simple command.

The simple command can be a **built in command** (cd or exit),
an **environment variable assignment**, or an **external command**.

1. **Fork and execve functionality**
In order to implement the fork and execve functionality, I used
the laboratory workflow as a starting point.
So I have this snippet of code:

**pid = fork();**
switch (pid) {
// Child process
case 0:
    
    // Redirect input/output/error

    **status = execvp(...);**
    if (status == -1)
        // Error handling

    exit(status);

// Parent process
default:
    ret = waitpid(pid, &status, 0);
    if (ret < 0)
        // Error handling

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    break;
}

Fork() function creates a new process for the child. 
The child process execute the command using execvp() function
and the parent process waits for the child to finish. In case the child exits
with an error, the parent process handles the error.

2. **Redirection of input/output/error**
I have implemented the redirection of input/output/error using **dup2()** function, 
in helpers.c -> redirects() function which handles the redirection of input/output/error.

For example, if the command is "ls > file.txt", file.txt will be opened, and 
a file descriptor will be assigned to it. Then, dup2() will assign a new file 
desciptor to the file, which will be the "standard" output/input/error for this command.

3. **Pipe functionality -> cmd1 | cmd2**
In order to implement the pipe functionality, I used pipe() function
(https://man7.org/linux/man-pages/man2/pipe.2.html), which is in charge of
creating a link between 2 file desciptors.

The main process starts two child processes.
The first child executes the first command and redirect his output
to the file descriptor assigned for writing (pipefd[1]).

The second child executes the second command, taking as input, the output
of the first command, which is the file descriptor assigned for reading.
(pipefd[0]).

4. **Parallel commands -> cmd1 & cmd2**
The parallel commands is implemented using 2 child processes,
which execute the 2 commands symultaneously. 
The fork, execve and waitpid are used in the same way as in the snippet of code described above.




