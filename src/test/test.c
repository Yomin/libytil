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

#include <ytil/test/test.h>
#include <ytil/test/ctx.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>


void _test_pos(void *vctx, test_pos_id type, const char *file, size_t line)
{
    test_ctx_st *ctx = vctx;
    
    if(test_com_send_duration(ctx->com, ctx->clock, &ctx->start)
    || test_com_send_position(ctx->com, type, file, line)
    || clock_gettime(ctx->clock, &ctx->start))
        abort();
}

void _test_msg(void *vctx, const char *file, size_t line, test_msg_id type, const char *msg, ...)
{
    test_ctx_st *ctx = vctx;
    va_list ap;
    
    va_start(ap, msg);
    
    if(test_com_send_duration(ctx->com, ctx->clock, &ctx->start)
    || test_com_send_position(ctx->com, TEST_POS_EXACT, file, line)
    || test_com_send_msg_v(ctx->com, type, msg, ap)
    || clock_gettime(ctx->clock, &ctx->start))
        abort();
    
    va_end(ap);
}

void _test_begin(void *ctx, const char *file, size_t line)
{
    _test_pos(ctx, TEST_POS_EXACT, file, line);
}

void _test_end(void *vctx, const char *file, size_t line)
{
    test_ctx_st *ctx = vctx;
    
    if(test_com_send_duration(ctx->com, ctx->clock, &ctx->start)
    || test_com_send_pass(ctx->com)
    || test_com_send_position(ctx->com, TEST_POS_AFTER, file, line)
    || clock_gettime(ctx->clock, &ctx->start))
        abort();
}

void _test_abort(void *vctx, const char *file, size_t line, const char *msg, ...)
{
    test_ctx_st *ctx = vctx;
    va_list ap;
    
    va_start(ap, msg);
    
    if(test_com_send_duration(ctx->com, ctx->clock, &ctx->start)
    || test_com_send_position(ctx->com, TEST_POS_EXACT, file, line)
    || test_com_send_msg_v(ctx->com, TEST_MSG_ERROR, msg, ap))
        abort();
    
    va_end(ap);
    
    if(ctx->jump)
        longjmp(*ctx->jump, 1);
    
    _exit(-1);
}

void *_test_alloc(void *ctx, const char *file, size_t line, size_t size)
{
    void *mem;
    
    if((mem = calloc(1, size)))
    {
        _test_pos(ctx, TEST_POS_AFTER, file, line);
        return mem;
    }
    else
    {
        _test_abort(ctx, file, line, "test alloc failed: %s", strerror(errno));
        return NULL;
    }
}

void _test_free(void *ctx, const char *file, size_t line, void *mem)
{
    _test_pos(ctx, TEST_POS_EXACT, file, line);
    
    free(mem);
}
