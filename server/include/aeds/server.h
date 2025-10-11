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

#ifndef SERVER_INCLUDE_AEDS_SERVER_H_
#define SERVER_INCLUDE_AEDS_SERVER_H_

#include <stddef.h>

#include "aeds/ret_types.h"

#define LIMIT_OF_INCOMING_CONNECTIONS 1

#define AESD_LOG_WITH_FUNC_DEBUG(msg, ...) syslog(LOG_DEBUG, "[%s] " msg, __func__, ##__VA_ARGS__)
#define AESD_LOG_WITH_FUNC_INFO(msg, ...) syslog(LOG_INFO, "[%s] " msg, __func__, ##__VA_ARGS__)
#define AESD_LOG_WITH_FUNC_ERR(msg, ...) syslog(LOG_ERR, "[%s] " msg, __func__, ##__VA_ARGS__)

typedef struct aesd_server_impl_s aesd_server_impl_t;

typedef struct aesd_server_s {
    aesd_server_impl_t * impl;
} aesd_server_t;

#ifdef __cplusplus
extern "C" {
#endif

// In C++, memory allocation and object initialization are separate steps. The
// C implementation of these steps is exposed so they can be used separately by
// the C++ interface, if necessary. Users of the C interface SHOULD NOT invoke
// these functions directly, but use aesd_server_create() and aesd_server_destroy() instead.
void * aesd_server_alloc(void);
void aesd_server_free(void * ptr);
aesd_server_t * aesd_server_init(aesd_server_t * aesd_server);
void aesd_server_fini(aesd_server_t * aesd_server);

// Creates a new object instance. Internally, this function invokes aesd_server_alloc()
// followed by aesd_server_init().
aesd_server_t * aesd_server_create();

// Destroys an object instance. Internally, this function invokes aesd_serverfini()
// followed by aesd_server_free().
void aesd_server_destroy(aesd_server_t * aesd_server);

void aesd_server_start_accept_connections(aesd_server_t * aesd_server);

/**
 * @brief Fills @a buf with a string that ends with '\n' sent by the client.
 * 
 * @param aesd_server 
 * @param buf Buffer to store the string
 * @param buf_len Length of the buffer
 * @param line_size If not null, store the length of the line read
 * @retval AESD_SERVER_RET_ERROR if some error ocurred
 * @retval AESD_SERVER_RET_EOL_FOUND if a string ending with '\n' was found and stored in buffer
 * @retval AESD_SERVER_RET_EOL_NOT_FOUND if all space of @buf was filled but none '\n' was not found.
 * In this case line_size = 0. The application code can call this function again to try retrieve
 * a string enfing in '\n'
 */
aesd_server_ret_t
aesd_server_get_line(aesd_server_t * aesd_server, void * buf, size_t buf_len, size_t * line_size);

#ifdef __cplusplus
}
#endif

#endif  // SERVER_INCLUDE_AEDS_SERVER_H_
