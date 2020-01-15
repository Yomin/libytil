/*
 * Copyright (c) 2019 Martin Rödel a.k.a. Yomin Nimoy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <ytil/test/com.h>
#include <ytil/def.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef _WIN32
#   include <sys/socket.h>
#endif

typedef struct test_com
{
    int sock;
    char *buf;
    size_t size, cap, pos;
    bool shortcut;
    test_com_msg_cb cb;
    void *ctx;
} test_com_st;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_TEST_COM_CALLBACK, "Callback error.")
    , ERROR_INFO(E_TEST_COM_INVALID_MSG_TYPE, "Invalid message type.")
    , ERROR_INFO(E_TEST_COM_INVALID_STATUS_TYPE, "Invalid status type.")
    , ERROR_INFO(E_TEST_COM_INVALID_RESULT_TYPE, "Invalid result type.")
    , ERROR_INFO(E_TEST_COM_INVALID_POS_TYPE, "Invalid position type.")
    , ERROR_INFO(E_TEST_COM_INVALID_TIMESTAMP, "Invalid timestamp.")
    , ERROR_INFO(E_TEST_COM_INVALID_TEXT_FORMAT, "Invalid text message format.")
    , ERROR_INFO(E_TEST_COM_NOT_AVAILABLE, "Function not available.")
    , ERROR_INFO(E_TEST_COM_SHUTDOWN, "Test case peer shutdown.")
    , ERROR_INFO(E_TEST_COM_WOULD_BLOCK, "Socket would block.")
};


test_com_ct test_com_new(test_com_msg_cb cb, void *ctx)
{
    test_com_ct com;
    
    if(!(com = calloc(1, sizeof(test_com_st))))
        return error_wrap_errno(calloc), NULL;
    
    com->cb = cb;
    com->ctx = ctx;
    
#ifdef _WIN32
    com->shortcut = true;
#endif
    
    return com;
}

void test_com_free(test_com_ct com)
{
    assert(com);
    
    if(com->buf)
        free(com->buf);
    
    if(com->sock >= 0)
        close(com->sock);
    
    free(com);
}

void test_com_reset(test_com_ct com)
{
    assert(com);
    
    if(com->sock >= 0)
        close(com->sock);
    
    com->sock = -1;
    com->size = 0;
    com->pos = 0;
}

void test_com_set_socket(test_com_ct com, int sock)
{
    assert(com);
    
    com->sock = sock;
}

int test_com_get_socket(test_com_ct com)
{
    assert(com);
    
    return com->sock;
}

int test_com_enable_shortcut(test_com_ct com, bool shortcut)
{
    assert(com);
    
#ifdef _WIN32
    return_error_if_fail(shortcut, E_TEST_COM_NOT_AVAILABLE, -1);
#endif
    
    com->shortcut = shortcut;
    
    return 0;
}

#ifndef _WIN32
static int test_com_write(test_com_ct com, void *vdata, size_t size)
{
    char *data = vdata;
    ssize_t count;
    
    for(; size; data += count, size -= count)
        if((count = send(com->sock, data, size, 0)) < 0)
            return error_wrap_errno(send), -1;
    
    return 0;
}

static int test_com_send(test_com_ct com, test_com_msg_id type, ...)
{
    va_list ap;
    void *data;
    size_t size;
    
    assert(type < TEST_COM_TYPES);
    
    if(test_com_write(com, &type, sizeof(test_com_msg_id)))
        return error_pass(), -1;
    
    va_start(ap, type);
    
    while((data = va_arg(ap, void*)))
    {
        size = va_arg(ap, size_t);
        
        if(test_com_write(com, data, size))
            return va_end(ap), error_pass(), -1;
    }
    
    va_end(ap);
    
    return 0;
}
#endif // !_WIN32

int test_com_send_status(test_com_ct com, test_status_id status)
{
    test_com_msg_un msg = { .status = status };
    
    assert(com);
    return_error_if_fail(status < TEST_STATUSES, E_TEST_COM_INVALID_STATUS_TYPE, -1);
    
#ifndef _WIN32
    if(!com->shortcut)
        return error_pass_int(test_com_send(com, TEST_COM_STATUS
            , &status, sizeof(test_status_id)
            , NULL));
#endif
    
    return error_push_int(E_TEST_COM_CALLBACK, com->cb(TEST_COM_STATUS, &msg, com->ctx));
}

int test_com_send_result(test_com_ct com, test_result_id result)
{
    test_com_msg_un msg = { .result = result };
    
    assert(com);
    return_error_if_fail(result < TEST_RESULTS, E_TEST_COM_INVALID_RESULT_TYPE, -1);
    
#ifndef _WIN32
    if(!com->shortcut)
        return error_pass_int(test_com_send(com, TEST_COM_RESULT
            , &result, sizeof(test_result_id)
            , NULL));
#endif
    
    return error_push_int(E_TEST_COM_CALLBACK, com->cb(TEST_COM_RESULT, &msg, com->ctx));
}

int test_com_send_duration(test_com_ct com, clockid_t clock, timespec_st *start)
{
    test_com_msg_un msg = {0};
    timespec_st now;
    
    assert(com);
    return_error_if_fail(start, E_TEST_COM_INVALID_TIMESTAMP, -1);
    
    clock_gettime(clock, &now);
    msg.duration = time_ts_diff_milli(start, &now);
    
#ifndef _WIN32
    if(!com->shortcut)
        return error_pass_int(test_com_send(com, TEST_COM_DURATION
            , &msg.duration, sizeof(size_t)
            , NULL));
#endif
    
    return error_push_int(E_TEST_COM_CALLBACK, com->cb(TEST_COM_DURATION, &msg, com->ctx));
}

int test_com_send_position(test_com_ct com, test_pos_id type, const char *file, size_t line)
{
    test_com_msg_un msg = { .pos.type = type, .pos.file = (char*)file, .pos.line = line };
    
    assert(com);
    return_error_if_fail(file && type < TEST_POS_TYPES, E_TEST_COM_INVALID_POS_TYPE, -1);
    
#ifndef _WIN32
    if(!com->shortcut)
    {
        size_t len = strlen(file) +1;
        
        return error_pass_int(test_com_send(com, TEST_COM_POS
            , &type, sizeof(test_pos_id)
            , &len, sizeof(size_t)
            , file, len
            , &line, sizeof(size_t)
            , NULL));
    }
#endif
    
    return error_push_int(E_TEST_COM_CALLBACK, com->cb(TEST_COM_POS, &msg, com->ctx));
}

int test_com_send_pass(test_com_ct com)
{
    assert(com);
    
#ifndef _WIN32
    if(!com->shortcut)
        return error_pass_int(test_com_send(com, TEST_COM_PASS, NULL));
#endif
    
    return error_push_int(E_TEST_COM_CALLBACK, com->cb(TEST_COM_PASS, NULL, com->ctx));
}

int test_com_send_msg(test_com_ct com, test_msg_id type, size_t level, const char *fmt, ...)
{
    va_list ap;
    int rc;
    
    va_start(ap, fmt);
    rc = error_pass_int(test_com_send_msg_v(com, type, level, fmt, ap));
    va_end(ap);
    
    return rc;
}

int test_com_send_msg_v(test_com_ct com, test_msg_id type, size_t level, const char *fmt, va_list ap1)
{
    test_com_msg_un msg = { .msg.type = type, .msg.level = level };
    va_list ap2;
    size_t len;
    
    assert(com);
    return_error_if_fail(fmt, E_TEST_COM_INVALID_TEXT_FORMAT, -1);
    
    va_copy(ap2, ap1);
    
    len = vsnprintf(NULL, 0, fmt, ap1) +1;
    msg.msg.text = alloca(len);
    vsnprintf(msg.msg.text, len, fmt, ap2);
    
    va_end(ap2);
    
#ifndef _WIN32
    if(!com->shortcut)
        return error_pass_int(test_com_send(com, TEST_COM_MSG
            , &type, sizeof(test_msg_id)
            , &level, sizeof(size_t)
            , &len, sizeof(size_t)
            , msg.msg.text, len
            , NULL));
#endif
    
    return error_push_int(E_TEST_COM_CALLBACK, com->cb(TEST_COM_MSG, &msg, com->ctx));
}

#ifndef _WIN32
static char *test_com_alloc(test_com_ct com, size_t size)
{
    size_t cap;
    char *tmp;
    
    if(!com->buf)
    {
        cap = MAX(size, 128U);
        
        if(!(com->buf = malloc(cap)))
            return error_wrap_errno(malloc), NULL;
        
        com->cap = cap;
        com->size = 0;
    }
    else if(com->size + size > com->cap)
    {
        cap = MAX(com->size+size, com->cap*2);
        
        if(!(tmp = realloc(com->buf, cap)))
            return error_wrap_errno(realloc), NULL;
        
        com->buf = tmp;
        com->cap = cap;
    }
    
    return com->buf + com->size;
}

static int test_com_read(test_com_ct com, void *dst, size_t dst_size)
{
    char *data;
    size_t size;
    ssize_t count;
    
    if(com->pos + dst_size > com->size)
    {
        size = com->pos + dst_size - com->size;
        
        if(!(data = test_com_alloc(com, size)))
            return error_pass(), -1;
        
        for(; size; data += count, size -= count, com->size += count)
            if((count = recv(com->sock, data, size, 0)) > 0)
                continue;
            else if(!count)
                return error_set(E_TEST_COM_SHUTDOWN), -1;
            else if(errno == EWOULDBLOCK)
                return error_set(E_TEST_COM_WOULD_BLOCK), -1;
            else
                return error_wrap_errno(recv), -1;
    }
    
    if(dst)
        memcpy(dst, &com->buf[com->pos], dst_size);
    
    com->pos += dst_size;
    
    return 0;
}

static int test_com_read_type(test_com_ct com, test_com_msg_id *type)
{
    return error_pass_int(test_com_read(com, type, sizeof(test_com_msg_id)));
}

static int test_com_read_status(test_com_ct com, test_status_id *status)
{
    return error_pass_int(test_com_read(com, status, sizeof(test_status_id)));
}

static int test_com_read_result(test_com_ct com, test_result_id *result)
{
    return error_pass_int(test_com_read(com, result, sizeof(test_result_id)));
}

static int test_com_read_pos_type(test_com_ct com, test_pos_id *type)
{
    return error_pass_int(test_com_read(com, type, sizeof(test_pos_id)));
}

static int test_com_read_msg_type(test_com_ct com, test_msg_id *type)
{
    return error_pass_int(test_com_read(com, type, sizeof(test_msg_id)));
}

static int test_com_read_size(test_com_ct com, size_t *size)
{
    return error_pass_int(test_com_read(com, size, sizeof(size_t)));
}

static int test_com_read_str(test_com_ct com, char **str)
{
    size_t size, offset;
    
    if(test_com_read(com, &size, sizeof(size_t)))
        return error_pass(), -1;
    
    offset = com->pos;
    
    if(test_com_read(com, NULL, size))
        return error_pass(), -1;
    
    *str = &com->buf[offset]; // fixme next alloc may invalidate str
    
    return 0;
}

static int test_com_recv_msg(test_com_ct com)
{
    test_com_msg_id type;
    test_com_msg_un msg = {0};
    int rc = 0;
    
    com->pos = 0;
    
    if(test_com_read_type(com, &type))
        return error_pass(), -1;
    
    switch(type)
    {
    case TEST_COM_STATUS:
        rc = test_com_read_status(com, &msg.status);
        break;
    case TEST_COM_RESULT:
        rc = test_com_read_result(com, &msg.result);
        break;
    case TEST_COM_DURATION:
        rc = test_com_read_size(com, &msg.duration);
        break;
    case TEST_COM_POS:
        if((rc = test_com_read_pos_type(com, &msg.pos.type))
        || (rc = test_com_read_str(com, &msg.pos.file))
        || (rc = test_com_read_size(com, &msg.pos.line)))
            break;
        break;
    case TEST_COM_PASS:
        break;
    case TEST_COM_MSG:
        if((rc = test_com_read_msg_type(com, &msg.msg.type))
        || (rc = test_com_read_size(com, &msg.msg.level))
        || (rc = test_com_read_str(com, &msg.msg.text)))
            break;
        break;
    default:
        return_error_if_reached(E_TEST_COM_INVALID_MSG_TYPE, -1);
    }
    
    if(rc)
        return error_pass(), rc;
    
    rc = error_push_int(E_TEST_COM_CALLBACK, com->cb(type, &msg, com->ctx));
    
    if(com->pos < com->size)
        memmove(com->buf, &com->buf[com->pos], com->size - com->pos);
    
    com->size -= com->pos;
    
    return rc;
}

int test_com_recv(test_com_ct com)
{
    int rc;
    
    assert(com);
    
    while(1)
        if(!(rc = test_com_recv_msg(com)))
            continue;
        else if(rc > 0)
            return rc;
        else if(error_check(0, E_TEST_COM_WOULD_BLOCK) || error_check(0, E_TEST_COM_SHUTDOWN))
            return 0;
        else
            return error_pass(), -1;
}
#endif // !_WIN32
