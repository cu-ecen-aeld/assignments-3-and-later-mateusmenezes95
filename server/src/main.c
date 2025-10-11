// Copyright (c) 2025 Mateus Menezes

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <aeds/server.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

struct read_buffer {
  char data[BUFFER_SIZE];
  size_t qty_bytes_read;
};

struct file_context {
  size_t file_size;
  int fd;
  int flags;
  mode_t mode;
};

static volatile sig_atomic_t sigint_or_sigterm_recved = 0;
static volatile sig_atomic_t sign_recved = 0;

static void
signal_handler(int sig) {
  sigint_or_sigterm_recved = 1;
  sign_recved = sig;
}

int main(void) {
  openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);
  struct sigaction sig_action;
  sig_action.sa_flags = 0;
  sig_action.sa_handler = signal_handler;

  if (sigaction(SIGINT, &sig_action, NULL) == -1) {
    syslog(LOG_ERR, "Error on setting sig handler for SIGINT: %s\n", strerror(errno));
    return -1;
  }

  if (sigaction(SIGTERM, &sig_action, NULL) == -1) {
    syslog(LOG_ERR, "Error on setting sig handler for SIGTERM: %s\n", strerror(errno));
    return -1;
  }

  struct file_context file_ctx;
  struct read_buffer buffer;
  memset(&file_ctx, 0, sizeof(struct file_context));

  file_ctx.flags = O_CREAT | O_RDWR | O_APPEND | O_TRUNC;
  file_ctx.mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
  file_ctx.fd = open("/var/tmp/aesdsocketdata", file_ctx.flags, file_ctx.mode);
  if (file_ctx.fd == -1) {
    syslog(LOG_ERR, "Error on creating the tmp file: %s\n", strerror(errno));
    closelog();
    return -1;
  }

  aesd_server_t * aesd_server;
  aesd_server = aesd_server_create();

  if (aesd_server != NULL) {
    syslog(LOG_INFO, "Server created sucessfully");
  }

  aesd_server_start_accept_connections(aesd_server);

  int get_line_ret = AESD_SERVER_RET_EOL_NOT_FOUND;
  size_t line_size;
  while (!sigint_or_sigterm_recved) {
    get_line_ret = aesd_server_get_line(aesd_server, buffer.data, BUFFER_SIZE, &line_size);
    if (get_line_ret == AESD_SERVER_RET_EOL_FOUND) {
      if (line_size + 1 < BUFFER_SIZE) buffer.data[line_size] = '\0';
      printf("Received: %s", buffer.data);
    }
  }

  if (sigint_or_sigterm_recved) {
    printf("Received %s\n", sign_recved == SIGINT ? "SIGINT" : "SIGTERM");
  }

  aesd_server_destroy(aesd_server);
  closelog();
  return 0;
}
