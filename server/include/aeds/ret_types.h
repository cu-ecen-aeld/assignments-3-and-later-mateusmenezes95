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

#ifndef SERVER_INCLUDE_AEDS_RET_TYPES_H_
#define SERVER_INCLUDE_AEDS_RET_TYPES_H_

#include <stdint.h>

typedef int32_t aesd_ret_t;

/// The operation ran as expected
#define AESD_RET_OK 0
/// Generic error to indicate operation could not complete successfully
#define AESD_RET_ERROR 1

typedef int32_t aesd_server_ret_t;

#define AESD_SERVER_RET_ERROR AESD_RET_ERROR
#define AESD_SERVER_RET_EOL_FOUND AESD_RET_OK
/// The read method does not found any '\n' in the buffer
#define AESD_SERVER_RET_EOL_NOT_FOUND 

#endif  // SERVER_INCLUDE_AEDS_RET_TYPES_H_
