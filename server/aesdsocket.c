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

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <netdb.h>
#include <unistd.h>

#define LIMIT_OF_INCOMING_CONNECTIONS 10
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

int main(void) {
  openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);

  struct addrinfo addrinfo_hints;
  struct addrinfo *servinfo;

  memset(&addrinfo_hints, 0, sizeof(addrinfo_hints));

  addrinfo_hints.ai_family = AF_INET;
  addrinfo_hints.ai_socktype = SOCK_STREAM;
  addrinfo_hints.ai_flags = AI_PASSIVE;

  int getaddrinfo_ret;
  if ((getaddrinfo_ret = getaddrinfo(NULL, "9000", &addrinfo_hints, &servinfo)) != 0) {
      syslog(LOG_ERR, "gai error: %s\n", gai_strerror(getaddrinfo_ret));
      return -1;
  }

  struct sockaddr_in *server_addr = (struct sockaddr_in *)servinfo->ai_addr;
  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(server_addr->sin_addr), ip_str, sizeof(ip_str));
  syslog(LOG_INFO, "Server address: %s:%hd\n", ip_str, ntohs(server_addr->sin_port));

  int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if (sockfd == -1) {
      syslog(LOG_ERR, "Socket creation error with errno: %s\n", strerror(errno));
      return -1;
  }

  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    syslog(LOG_ERR, "Error trying to configuring the socket: %s", strerror(errno));
    close(sockfd);
    return -1;
  }

  if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
      syslog(LOG_ERR, "Bind error with errno: %s\n", strerror(errno));
      close(sockfd);
      return -1;
  }

  freeaddrinfo(servinfo);

  if (listen(sockfd, LIMIT_OF_INCOMING_CONNECTIONS) == -1) {
    syslog(LOG_ERR, "Error on listening: %s", strerror(errno));
    close(sockfd);
    return -1;
  }

  struct sockaddr_storage client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int connection_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (connection_fd == -1) {
    syslog(LOG_ERR, "Error on accepting new connection: %s", strerror(errno));
    close(sockfd);
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

  while (1) {
    buffer.qty_bytes_read = recv(connection_fd, &buffer.data, BUFFER_SIZE, 0);
    if (buffer.qty_bytes_read == -1) {
      if (errno == EINTR) continue;  // Retry if interrupted by signal
      syslog(LOG_ERR, "Error on receiving data: %s", strerror(errno));
      close(connection_fd);
      close(sockfd);
      return -1;
    }

    char * first_end_str;
    first_end_str = strchr(buffer.data, '\n');
    if (first_end_str != NULL) {
      file_ctx.file_size += buffer.qty_bytes_read;
      int str_size = first_end_str - buffer.data;
      ssize_t bytes_written = write(file_ctx.fd, buffer.data, str_size);
      if (bytes_written < str_size) {
        syslog(LOG_WARNING, "Partial written. Expected to write %d, but only %ld was written",
        str_size, bytes_written);
      }
      memset(&buffer, 0, sizeof(struct read_buffer));
    }
  }

  close(file_ctx.fd);
  close(connection_fd);
  close(sockfd);

  closelog();
  return 0;
}
