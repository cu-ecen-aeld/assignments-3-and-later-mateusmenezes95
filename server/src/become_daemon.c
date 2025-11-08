/************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2019.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/*
 * Modifications to the original work:
 *   - Date: 2025-10-21
 *   - Author: mateusmenezes95
 *   - Changes:
 *          1. Removed flag definitions from lines 22, 23, 24, and 26
 *             (BD_NO_CHDIR, BD_NO_CLOSE_FILES, BD_NO_REOPEN_STD_FDS, BD_NO_UMASK0)
 *             to use the default daemon behavior.
 *           2. Added pipe signaling mechanism
 *   - Original source: https://github.com/bradfa/tlpi-dist/blob/a00ffc86b77ef407792a9bbd87f39326ba6dd481/daemons/become_daemon.h
 *   - License: This modified file remains under GNU LGPL v3 or later (see COPYING.lgpl-v3)
 *
 * Note: This is a private submission for a Coursera course exercise.
 */


/* Listing 37-2 */

/* become_daemon.c

   A function encapsulating the steps in becoming a daemon.
*/
#include "aeds/become_daemon.h"

#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>


int                                     /* Returns 0 on success, -1 on error */
becomeDaemon()
{
    int maxfd, fd;
    int pipefd[2];
    char unit_buf;

    if (pipe(pipefd) == -1) {
        return -1;
    }

    switch (fork()) {                   // Become background process
        case -1: return -1;
        case 0:
            close(pipefd[0]);           // Child closes read end
            break;                      // Child falls through...
        default:
            close(pipefd[1]);           // Parent closes write end
            read(pipefd[0], &unit_buf, 1); // Wait for grandchild signal
            close(pipefd[0]);
            _exit(EXIT_SUCCESS);        // Parent terminates
    }

    if (setsid() == -1)                 // Become leader of new session
        return -1;

    switch (fork()) {                   // Ensure we are not session leader
        case -1: return -1;
        case 0:  
            // Grandchild keeps pipefd[1] open to signal original parent
            break;
        default: 
            // Second child exits immediately
            close(pipefd[1]);           // Close pipe before exiting
            _exit(EXIT_SUCCESS);
    }

    // Only the grandchild (final daemon) reaches here with pipefd[1] still open

    maxfd = sysconf(_SC_OPEN_MAX);
    if (maxfd == -1) {
        maxfd = BD_MAX_CLOSE;
    }

    // Close all file descriptors EXCEPT the pipe write end
    for (fd = 0; fd < maxfd; fd++) {
        if (fd != pipefd[1]) {
            close(fd);
        }
    }

    close(STDIN_FILENO);

    fd = open("/dev/null", O_RDWR);

    if (fd != STDIN_FILENO) {
        return -1;
    }

    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
        return -1;
    }

    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
        return -1;
    }

    return pipefd[1];  // Return pipe write end to signal parent
}
