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
#include <errno.h>


void _test_pos(void *vctx, test_pos_id type, const char *file, size_t line)
{
    test_ctx_st *ctx = vctx;
    
    if(test_com_send_duration(ctx->com, ctx->clock, &ctx->start)
    || test_com_send_position(ctx->com, type, file, line)
    || clock_gettime(ctx->clock, &ctx->start))
        abort();
}

static void _test_msg_v(void *vctx, const char *file, size_t line, test_msg_id type, size_t level, bool backtrace, const char *msg, va_list ap)
{
    test_ctx_st *ctx = vctx;
    size_t e;
    
    if(test_com_send_duration(ctx->com, ctx->clock, &ctx->start)
    || test_com_send_position(ctx->com, TEST_POS_EXACT, file, line)
    || test_com_send_msg_v(ctx->com, type, level, msg, ap)
    || clock_gettime(ctx->clock, &ctx->start))
        abort();
    
    if(backtrace)
        for(e=0; e < error_depth(); e++)
        {
            if(error_stack_get_type(e) == &ERROR_TYPE_GENERIC)
                switch(error_stack_get_code(e))
                {
                case E_GENERIC_INVALID:
                    abort();
                case E_GENERIC_WRAP:
                    test_com_send_msg(ctx->com, type, 1, "%02zu %s: <wrap>",
                        e, error_stack_get_func(e));
                    continue;
                case E_GENERIC_PASS:
                    test_com_send_msg(ctx->com, type, 1, "%02zu %s: <pass>",
                        e, error_stack_get_func(e));
                    continue;
                case E_GENERIC_SKIP:
                    test_com_send_msg(ctx->com, type, 1, "%02zu %s: <skip>",
                        e, error_stack_get_func(e));
                    continue;
                default:
                    break;
                }

            test_com_send_msg(ctx->com, type, 1, "%02zu %s: %s",
                e, error_stack_get_func(e), error_stack_get_name(e));
            test_com_send_msg(ctx->com, type, 2, "%s",
                error_stack_get_desc(e));
        }
}

void _test_msg(void *ctx, const char *file, size_t line, test_msg_id type, size_t level, bool backtrace, const char *msg, ...)
{
    va_list ap;
    
    va_start(ap, msg);
    _test_msg_v(ctx, file, line, type, level, backtrace, msg, ap);
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

void _test_abort(void *vctx, const char *file, size_t line, bool backtrace, const char *msg, ...)
{
    test_ctx_st *ctx = vctx;
    va_list ap;
    
    va_start(ap, msg);
    _test_msg_v(ctx, file, line, TEST_MSG_ERROR, 0, backtrace, msg, ap);
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
