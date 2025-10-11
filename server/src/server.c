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

#include "aeds/server.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include "aeds/ret_types.h"

struct aesd_server_impl_s {
    int socket_fd;
    int connection_fd;
    bool connection_accepted;
};

void *
aesd_server_alloc(void)
{
    openlog("aesd_server_lib", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_DEBUG, "Trying to allocate memory for the aesd_server_impl_s struct");

    aesd_server_t * outer_struct_ptr = (aesd_server_t *)malloc(sizeof(struct aesd_server_s));
    if (!outer_struct_ptr) {
        syslog(LOG_ERR, "Error during memory allocation: %s", strerror(errno));
    } else {
        syslog(LOG_DEBUG, "Memory allocated to outer structure sucessfully");
        memset(outer_struct_ptr, 0, sizeof(struct aesd_server_s));
    }

    outer_struct_ptr->impl = (struct aesd_server_impl_s *)malloc(sizeof(struct aesd_server_impl_s));
    if (!outer_struct_ptr->impl) {
        syslog(LOG_ERR, "Error during memory allocation: %s", strerror(errno));
    } else {
        syslog(LOG_DEBUG, "Memory allocated to inner structure sucessfully");
        memset(outer_struct_ptr->impl, 0, sizeof(struct aesd_server_impl_s));
    }

    return outer_struct_ptr;
}

void
aesd_server_free(void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    aesd_server_t * server = (aesd_server_t *)ptr;

    if (server->impl != NULL) {
        free(server->impl);
    }

    free(ptr);
}

aesd_server_t *
aesd_server_init(aesd_server_t * aesd_server)
{
    assert(aesd_server);

    if (aesd_server == NULL) {
        syslog(LOG_ERR, "Pointer passed to aesd_server_init is NULL");
        return NULL;
    }

    aesd_server->impl->socket_fd = -1;
    aesd_server->impl->connection_fd = -1;
    aesd_server->impl->connection_accepted = false;

    struct addrinfo addrinfo_hints;
    struct addrinfo *servinfo;

    memset(&addrinfo_hints, 0, sizeof(addrinfo_hints));

    addrinfo_hints.ai_family = AF_INET;
    addrinfo_hints.ai_socktype = SOCK_STREAM;
    addrinfo_hints.ai_flags = AI_PASSIVE;

    int getaddrinfo_ret;
    if ((getaddrinfo_ret = getaddrinfo(NULL, "9000", &addrinfo_hints, &servinfo)) != 0) {
        syslog(LOG_ERR, "gai error: %s\n", gai_strerror(getaddrinfo_ret));
    }

    struct sockaddr_in *server_addr = (struct sockaddr_in *)servinfo->ai_addr;
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server_addr->sin_addr), ip_str, sizeof(ip_str));
    syslog(LOG_INFO, "Server address: %s:%hd\n", ip_str, ntohs(server_addr->sin_port));

    aesd_server->impl->socket_fd = socket(
        servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (aesd_server->impl->socket_fd == -1) {
        syslog(LOG_ERR, "Socket creation error with errno: %s\n", strerror(errno));
    }

    int yes = 1;
    int ret = setsockopt(aesd_server->impl->socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (ret == -1) {
        syslog(LOG_ERR, "Error trying to configuring the socket: %s", strerror(errno));
        goto close_socket;
    }

    if (bind(aesd_server->impl->socket_fd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        syslog(LOG_ERR, "Bind error with errno: %s\n", strerror(errno));
        goto close_socket;
    }

    freeaddrinfo(servinfo);

    if (listen(aesd_server->impl->socket_fd, LIMIT_OF_INCOMING_CONNECTIONS) == -1) {
        syslog(LOG_ERR, "Error on listening: %s", strerror(errno));
        goto close_socket;
    }

    return aesd_server;

close_socket:
    close(aesd_server->impl->socket_fd);
    if (aesd_server->impl->connection_accepted) {
        close(aesd_server->impl->connection_fd);
    }

    return NULL;
}

void
aesd_server_start_accept_connections(aesd_server_t * aesd_server)
{
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    aesd_server->impl->connection_fd = accept(
        aesd_server->impl->socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);

    if (aesd_server->impl->connection_fd == -1) {
        if (EINTR == errno) {
            syslog(LOG_INFO, "Accept connection interrupted by a signal");
        }

        syslog(LOG_ERR, "Error on accepting new connection: %s", strerror(errno));

        aesd_server->impl->connection_accepted = false;
        close(aesd_server->impl->socket_fd);
        return;
    }

    syslog(LOG_INFO, "AESD Server ready to receive data");
    aesd_server->impl->connection_accepted = true;
}

void
aesd_server_fini(aesd_server_t * aesd_server)
{
    assert(aesd_server);

    close(aesd_server->impl->socket_fd);
    close(aesd_server->impl->connection_fd);
}

aesd_server_t *
aesd_server_create()
{
    aesd_server_t * aesd_server = aesd_server_alloc();
    if (!aesd_server) {
        goto error_alloc;
    }

    aesd_server_t * tmp = aesd_server_init(aesd_server);
    if (!tmp) {
        goto error_init;
    }
    aesd_server = tmp;

    return aesd_server;

error_init:
    aesd_server_free(aesd_server);
error_alloc:
    return NULL;
}

void
aesd_server_destroy(aesd_server_t * aesd_server)
{
    // aesd_server_destroy() and aesd_server_free() are the only methods which can be called
    // with `aesd_server == NULL`.
    if (aesd_server) {
        aesd_server_fini(aesd_server);
        aesd_server_free(aesd_server);
    }
}

aesd_server_ret_t
aesd_server_get_line(aesd_server_t * aesd_server, void * buf, size_t buf_len, size_t * line_size) {
    ssize_t num_bytes_read = 0;
    size_t total_bytes_read = 0;
    char * first_end_str;
    char * this_buf = buf;

    assert(line_size);

    if (aesd_server == NULL) {
        AESD_LOG_WITH_FUNC_ERR("server is not properly initialized. Passed a null pointer");
        return -1;
    }

    total_bytes_read = 0;
    while (total_bytes_read < buf_len) {
        num_bytes_read = recv(aesd_server->impl->connection_fd,
            this_buf + num_bytes_read, buf_len - total_bytes_read, 0);
        if (num_bytes_read == 0) {
            return AESD_SERVER_RET_NO_BYTES_READ;
        }

        if (num_bytes_read == -1) {
            if (errno == EINTR) {
                syslog(LOG_INFO, "recv interrupted due to signal handling\n");
                return AESD_SERVER_RET_ERROR;
            }
            syslog(LOG_ERR, "Error during recv call: %s\n", strerror(errno));
            return AESD_SERVER_RET_ERROR;
        }

        first_end_str = memchr(buf, '\n', num_bytes_read);
        if (first_end_str != NULL) {
            *line_size = first_end_str - this_buf;
            AESD_LOG_WITH_FUNC_DEBUG("End of line found at buf[%ld]", *line_size);
            return AESD_SERVER_RET_EOL_FOUND;
        }

        if (buf_len == num_bytes_read) {
            return AESD_SERVER_RET_BUF_FULL;
        }

        total_bytes_read += num_bytes_read;
    }

    *line_size = 0;

    return AESD_SERVER_RET_EOL_NOT_FOUND;
}

aesd_server_ret_t
aesd_server_send_file_content(aesd_server_t * aesd_server, int file_fd) {
    if (file_fd < 0) {
        AESD_LOG_WITH_FUNC_ERR("Invalid file descriptor passed. Value is %d", file_fd);
        return AESD_SERVER_RET_ERROR;
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) == -1) {
        AESD_LOG_WITH_FUNC_ERR("File does not exist or cannot be accessed: %s", strerror(errno));
        return AESD_SERVER_RET_ERROR;
    }

    off_t file_offset = 0;  // Always read from the begin of the file
    int ret = sendfile(aesd_server->impl->connection_fd, file_fd, &file_offset, file_stat.st_size);

    if (ret == -1) {
        AESD_LOG_WITH_FUNC_ERR("Error on transferring data between fds: %s", strerror(errno));
        return AESD_SERVER_RET_ERROR;
    }

    if (ret != file_stat.st_size) {
        AESD_LOG_WITH_FUNC_ERR(
            "Expected to transfer %ld, but only %d was transferred", file_stat.st_size, ret);
        return AESD_SERVER_RET_ERROR;
    }

    return AESD_SERVER_RET_OK;
}
