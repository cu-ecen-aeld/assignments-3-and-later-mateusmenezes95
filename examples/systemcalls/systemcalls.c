#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

    if (cmd == NULL)
    {
        return false;
    }

    int ret = system(cmd);

    if (ret == -1) 
    {
        perror("do_system");     
    }

    if (ret != EXIT_SUCCESS)
    {
        return false;
    }

    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];

    printf("Command passed: ");
    for(int i = 0; i < count; i++)
    {
        command[i] = va_arg(args, char *);
        printf("%s ", command[i]);
    }

    printf("\n");
    command[count] = NULL;

    int status;
    pid_t child_pid;

    child_pid = fork();

    if (child_pid == -1)
    {
        perror("do_exec");
        return false;
    }

    if (child_pid == 0)
    {
        execv(command[0], command);        
        perror("do_exec");
        exit(EXIT_FAILURE);
    }

    if (waitpid(child_pid, &status, 0) == -1)
    {
        printf("Problem with waitpid");
        return false;
    }

    if (WIFEXITED(status))
    {
        printf("Child process %d terminated normally with status %d!\n",
            child_pid, WEXITSTATUS(status));
        if (WEXITSTATUS(status) != 0)
        {
            printf("Process terminated with non-zero status: %d\n", WEXITSTATUS(status));
            return false;
        }
        return true;
    }

    va_end(args);

    return false;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0)
    {
        perror("open");
        return false;
    }

    int status;
    pid_t child_pid;

    child_pid = fork();

    if (child_pid == -1)
    {
        perror("do_exec");
        return false;
    }

    if (child_pid == 0)
    {
        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd);

        execv(command[0], command);

        perror("do_exec");
        exit(EXIT_FAILURE);
    }

    if (waitpid(child_pid, &status, 0) == -1)
    {
        return false;
    }

    if (WIFEXITED(status))
    {
        if (WEXITSTATUS(status) != 0)
        {
            return false;
        }
        return true;
    }

    va_end(args);

    return false;
}
