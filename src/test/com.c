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
#include <sys/socket.h>
#include <sys/ioctl.h>


typedef struct test_com
{
    int sock;
    char *buf;
    size_t size, cap, pos;
    bool shortcut;
    test_com_msg_cb cb;
    void *ctx;
} test_com_st;


test_com_ct test_com_new(test_com_msg_cb cb, void *ctx)
{
    test_com_ct com;
    
    if(!(com = calloc(1, sizeof(test_com_st))))
        return NULL;
    
    com->cb = cb;
    com->ctx = ctx;
    
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

void test_com_enable_shortcut(test_com_ct com, bool shortcut)
{
    assert(com);
    
    com->shortcut = shortcut;
}

static int test_com_write(test_com_ct com, void *vdata, size_t size)
{
    char *data = vdata;
    ssize_t count;
    
    for(; size; data += count, size -= count)
        if((count = send(com->sock, data, size, 0)) < 0)
            return -1;
    
    return 0;
}

static int test_com_send(test_com_ct com, test_com_msg_id type, size_t n, ...)
{
    va_list ap;
    void *data;
    size_t size;
    
    return_err_if_fail(type < TEST_COM_TYPES, EINVAL, -1);
    
    if(test_com_write(com, &type, sizeof(test_com_msg_id)))
        return -1;
    
    va_start(ap, n);
    
    for(; n; n--)
    {
        data = va_arg(ap, void*);
        size = va_arg(ap, size_t);
        
        if(test_com_write(com, data, size))
            return va_end(ap), -1;
    }
    
    va_end(ap);
    
    return 0;
}

int test_com_send_status(test_com_ct com, test_status_id status)
{
    test_com_msg_un msg = { .status = status };
    
    assert(com);
    return_err_if_fail(status < TEST_STATUSES, EINVAL, -1);
    
    if(com->shortcut)
        return com->cb(TEST_COM_STATUS, &msg, com->ctx);
    
    return test_com_send(com, TEST_COM_STATUS, 1, &status, sizeof(test_status_id));
}

int test_com_send_result(test_com_ct com, test_result_id result)
{
    test_com_msg_un msg = { .result = result };
    
    assert(com);
    return_err_if_fail(result < TEST_RESULTS, EINVAL, -1);
    
    if(com->shortcut)
        return com->cb(TEST_COM_RESULT, &msg, com->ctx);
    
    return test_com_send(com, TEST_COM_RESULT, 1, &result, sizeof(test_result_id));
}

int test_com_send_duration(test_com_ct com, clockid_t clock, timespec_st *start)
{
    test_com_msg_un msg = {0};
    timespec_st now;
    
    assert(com);
    return_err_if_fail(start, EINVAL, -1);
    
    clock_gettime(clock, &now);
    msg.duration = time_ts_diff_milli(start, &now);
    
    if(com->shortcut)
        return com->cb(TEST_COM_DURATION, &msg, com->ctx);
    
    return test_com_send(com, TEST_COM_DURATION, 1, &msg.duration, sizeof(size_t));
}

int test_com_send_position(test_com_ct com, test_pos_id type, const char *file, size_t line)
{
    test_com_msg_un msg = { .pos.type = type, .pos.file = (char*)file, .pos.line = line };
    size_t len;
    
    assert(com);
    return_err_if_fail(file && type < TEST_POS_TYPES, EINVAL, -1);
    
    if(com->shortcut)
        return com->cb(TEST_COM_POS, &msg, com->ctx);
    
    len = strlen(file) +1;
    
    return test_com_send(com, TEST_COM_POS, 4,
        &type, sizeof(test_pos_id),
        &len, sizeof(size_t), file, len,
        &line, sizeof(size_t));
}

int test_com_send_pass(test_com_ct com)
{
    assert(com);
    
    if(com->shortcut)
        return com->cb(TEST_COM_PASS, NULL, com->ctx);
    
    return test_com_send(com, TEST_COM_PASS, 0);
}

int test_com_send_msg(test_com_ct com, test_msg_id type, const char *fmt, ...)
{
    va_list ap;
    int rc;
    
    va_start(ap, fmt);
    rc = test_com_send_msg_v(com, type, fmt, ap);
    va_end(ap);
    
    return rc;
}

int test_com_send_msg_v(test_com_ct com, test_msg_id type, const char *fmt, va_list ap1)
{
    test_com_msg_un msg = { .msg.type = type };
    va_list ap2;
    size_t len;
    
    assert(com);
    return_err_if_fail(fmt, EINVAL, -1);
    
    va_copy(ap2, ap1);
    
    len = vsnprintf(NULL, 0, fmt, ap1) +1;
    msg.msg.text = alloca(len);
    vsnprintf(msg.msg.text, len, fmt, ap2);
    
    va_end(ap2);
    
    if(com->shortcut)
        return com->cb(TEST_COM_MSG, &msg, com->ctx);
    
    return test_com_send(com, TEST_COM_MSG, 3,
        &type, sizeof(test_msg_id), &len, sizeof(size_t), msg.msg.text, len);
}

static char *test_com_alloc(test_com_ct com, size_t size)
{
    size_t cap;
    char *tmp;
    
    if(!com->buf)
    {
        cap = MAX(size, 128U);
        
        if(!(com->buf = malloc(cap)))
            return NULL;
        
        com->cap = cap;
        com->size = 0;
    }
    else if(com->size + size > com->cap)
    {
        cap = MAX(com->size+size, com->cap*2);
        
        if(!(tmp = realloc(com->buf, cap)))
            return NULL;
        
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
            return -1;
        
        for(; size; data += count, size -= count, com->size += count)
            if((count = recv(com->sock, data, size, 0)) < 0)
                return -1;
            else if(!count)
                return errno = ESHUTDOWN, -1;
    }
    
    if(dst)
        memcpy(dst, &com->buf[com->pos], dst_size);
    
    com->pos += dst_size;
    
    return 0;
}

static int test_com_read_type(test_com_ct com, test_com_msg_id *type)
{
    return test_com_read(com, type, sizeof(test_com_msg_id));
}

static int test_com_read_status(test_com_ct com, test_status_id *status)
{
    return test_com_read(com, status, sizeof(test_status_id));
}

static int test_com_read_result(test_com_ct com, test_result_id *result)
{
    return test_com_read(com, result, sizeof(test_result_id));
}

static int test_com_read_pos_type(test_com_ct com, test_pos_id *type)
{
    return test_com_read(com, type, sizeof(test_pos_id));
}

static int test_com_read_msg_type(test_com_ct com, test_msg_id *type)
{
    return test_com_read(com, type, sizeof(test_msg_id));
}

static int test_com_read_size(test_com_ct com, size_t *size)
{
    return test_com_read(com, size, sizeof(size_t));
}

static int test_com_read_str(test_com_ct com, char **str)
{
    size_t size, offset;
    
    if(test_com_read(com, &size, sizeof(size_t)))
        return -1;
    
    offset = com->pos;
    
    if(test_com_read(com, NULL, size))
        return -1;
    
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
        return -1;
    
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
        || (rc = test_com_read_str(com, &msg.msg.text)))
            break;
        break;
    default:
        return_err_if_reached(EPROTO, -1);
    }
    
    if(rc)
        return rc;
    
    rc = com->cb(type, &msg, com->ctx);
    
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
        switch(rc = test_com_recv_msg(com))
        {
        case  0: continue;
        case -1: return errno == EINTR || errno == EAGAIN || errno == ESHUTDOWN ? 0 : -1;
        default: return rc;
        }
}
