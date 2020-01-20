/*
 * Copyright (c) 2018-2021 Martin Rödel aka Yomin
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

#ifndef YTIL_PARSER_PARSER_H_INCLUDED
#define YTIL_PARSER_PARSER_H_INCLUDED

#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ytil/gen/str.h>
//#include <ytil/type/type.h>


/// parser error
typedef enum parser_error
{
    E_PARSER_INVALID_NAME
} parser_error_id;

/// parser error type declaration
ERROR_DECLARE(PARSER);

typedef enum parser_show
{
      PARSER_SHOW_DEFAULT
    , PARSER_SHOW_TOP
    , PARSER_SHOW_GROUP
} parser_show_id;

struct parser;
typedef struct parser       *parser_ct;
typedef const struct parser *parser_const_ct;
typedef struct parser_stack *parser_stack_ct;


typedef ssize_t (*parser_parse_fold_cb)(parser_const_ct p, const char *input, size_t size, bool result, parser_stack_ct stack, void *parse_ctx);
typedef ssize_t (*parser_parse_cb)(const char *input, size_t size, bool result, void *parser_ctx, parser_stack_ct stack, parser_parse_fold_cb parse, void *parse_ctx);
typedef int     (*parser_show_fold_cb)(parser_const_ct p, parser_show_id mode, void *state);
typedef int     (*parser_show_cb)(str_ct str, parser_show_id mode, void *ctx, parser_show_fold_cb show, void *state);

typedef void    (*parser_dtor_cb)(void *ctx);


//type_id   parser_type(void);

// create new parser
// parse is mandatory, show is optional
parser_ct parser_new(parser_parse_cb parse, parser_show_cb show, void *ctx, parser_dtor_cb dtor);
// free parser
void      parser_free(parser_ct p);

// set parser name
parser_ct parser_set_name(parser_ct p, str_const_ct name);
parser_ct parser_set_show(parser_ct p, parser_show_cb show);

// define parser with name, used for parser building, frees parser on error
parser_ct parser_define(str_const_ct name, parser_ct p);

//str_ct parser_show(parser_const_ct p);

ssize_t parser_parse(parser_const_ct p, const char *input, size_t len, size_t n, ...); // type_id type, void *result
ssize_t parser_parse_v(parser_const_ct p, const char *input, size_t len, size_t n, va_list ap);
ssize_t parser_parse_x(parser_const_ct p, const char *input, size_t len, void *ctx, size_t n, ...); // type_id type, void *result
ssize_t parser_parse_vx(parser_const_ct p, const char *input, size_t len, void *ctx, size_t n, va_list ap);

//int    parser_stack_push(parser_stack_ct stack, type_id type, const void *result);
//int    parser_stack_push_p(parser_stack_ct stack, type_id type, const void *result);

//int    parser_stack_pop(parser_stack_ct stack, type_id type, void *result);
//void  *parser_stack_pop_p(parser_stack_ct stack, type_id type);
//int    parser_stack_get(parser_stack_ct stack, size_t pos, type_id type, void *result);
//void  *parser_stack_get_p(parser_stack_ct stack, size_t pos, type_id type);

int    parser_stack_drop(parser_stack_ct stack, size_t n);

//type_id parser_stack_type(parser_stack_ct stack, size_t pos);
size_t  parser_stack_size(parser_stack_ct stack, size_t pos);
str_ct  parser_stack_show(parser_stack_ct stack, str_ct str, size_t pos);

size_t parser_stack_results(parser_stack_ct stack);
void   parser_stack_clear(parser_stack_ct stack);

/*
#ifndef __PARSER_INTERN__
#   include <ytil/parser/zero.h>
#   include <ytil/parser/char.h>
#   include <ytil/parser/str.h>
#   include <ytil/parser/num.h>
#   include <ytil/parser/combinator.h>
#   include <ytil/parser/logic.h>
#   include <ytil/parser/repeat.h>
#   include <ytil/parser/block.h>
#   include <ytil/parser/builder.h>
#   include <ytil/parser/con.h>
#endif
*/

#endif
