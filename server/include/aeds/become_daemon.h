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
 *   - Changes: Removed flag definitions from lines 22, 23, 24, and 26
 *              (BD_NO_CHDIR, BD_NO_CLOSE_FILES, BD_NO_REOPEN_STD_FDS, BD_NO_UMASK0)
 *              to restore default daemon behavior for Coursera exercise requirements.
 *   - Original source: https://github.com/bradfa/tlpi-dist/blob/a00ffc86b77ef407792a9bbd87f39326ba6dd481/daemons/become_daemon.h
 *   - License: This modified file remains under GNU LGPL v3 or later (see COPYING.lgpl-v3)
 *
 * Note: This is a private submission for a Coursera course exercise.
 */

/* Listing 37-1 */

/* become_daemon.h

   Header file for become_daemon.c.
*/
#ifndef BECOME_DAEMON_H             /* Prevent double inclusion */
#define BECOME_DAEMON_H

#define BD_MAX_CLOSE  8192          /* Maximum file descriptors to close if
                                       sysconf(_SC_OPEN_MAX) is indeterminate */

int becomeDaemon();

#endif
