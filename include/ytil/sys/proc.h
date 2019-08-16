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

#ifndef __YTIL_SYS_PROC_H__
#define __YTIL_SYS_PROC_H__

#include <stdarg.h>

typedef enum proc_error
{
      E_PROC_INVALID_TITLE
    , E_PROC_NO_SPACE
    , E_PROC_NOT_INITIALIZED
} proc_error_id;


// initialize proc title by scavenging argv and environ
int proc_init_title(int argc, char *argv[]);
// free proc title, bricks argv and environ
void proc_free_title(void);

// set process title
int proc_set_title(const char *fmt, ...) __attribute__((format (gnu_printf, 1, 2)));
// set process title, va_list version
int proc_set_title_v(const char *fmt, va_list ap) __attribute__((format (gnu_printf, 1, 0)));

// append to original process title
int proc_append_title(const char *fmt, ...) __attribute__((format (gnu_printf, 1, 2)));
// append to original process title, va_list version
int proc_append_title_v(const char *fmt, va_list ap) __attribute__((format (gnu_printf, 1, 0)));

#endif
