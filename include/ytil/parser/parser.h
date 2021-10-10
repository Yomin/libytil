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

/// \file

#ifndef YTIL_PARSER_PARSER_H_INCLUDED
#define YTIL_PARSER_PARSER_H_INCLUDED

#include <ytil/gen/error.h>
#include <sys/types.h>


/// parser error
typedef enum parser_error
{
    E_PARSER_FAIL,              ///< parser failed, another parser may succeed
    E_PARSER_ABORT,             ///< parser aborted, stop parsing altogether
    E_PARSER_ERROR,             ///< parser aborted with error
    E_PARSER_STACK_EMPTY,       ///< parse stack is empty
    E_PARSER_STACK_TYPE,        ///< wrong stack type requested
    E_PARSER_ARG_MISSING,       ///< missing parser argument on stack
    E_PARSER_ARG_TYPE,          ///< parser argument with wrong type on stack
    E_PARSER_RESULT_MISSING,    ///< missing parser result on stack
    E_PARSER_RESULT_TYPE,       ///< parser result with wrong type on stack
} parser_error_id;

/// parser error type declaration
ERROR_DECLARE(PARSER);

struct parser;
typedef       struct parser *parser_ct;         ///< parser type
typedef const struct parser *parser_const_ct;   ///< const parser type
typedef struct parser_stack *parser_stack_ct;   ///< parser stack type


/// parser parse callback
///
/// \param input    input
/// \param len      \p input length in bytes
/// \param ctx      parser context
/// \param stack    parse stack, if NULL no results are to be produced
///
/// \returns                            number of bytes parsed
/// \retval -1/E_PARSER_FAIL            parser failed
/// \retval -1/E_PARSER_ABORT           parser aborted
/// \retval -1/E_PARSER_ERROR           parser aborted with error
/// \retval -1/E_PARSER_ARG_MISSING     missing parser argument on stack
/// \retval -1/E_PARSER_ARG_TYPE        parser argument with wrong type on stack
/// \retval -1/E_PARSER_RESULT_MISSING  missing parser result on stack
/// \retval -1/E_PARSER_RESULT_TYPE     parser result with wrong type on stack
typedef ssize_t (*parser_parse_cb)(const void *input, size_t len, void *ctx, parser_stack_ct stack);

/// parser callback context dtor callback
///
/// \param ctx  context to destroy
typedef void (*parser_dtor_cb)(void *ctx);


/// Create new parser with a floating reference.
///
/// \note If this function fails, the callback context is immediately destroyed.
///
/// \param parse    parse callback
/// \param ctx      callback context
/// \param dtor     \p ctx dtor
///
/// \returns                    new parser
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_ct parser_new(parser_parse_cb parse, const void *ctx, parser_dtor_cb dtor);

/// Increase reference count of parser.
///
/// \param p    parser, may be NULL
///
/// \returns    p
parser_ct parser_ref(parser_ct p);

/// Decrease reference count of parser and free if no longer referenced.
///
/// \param p    parser, must not be NULL
///
/// \retval p       parser is still referenced
/// \retval NULL    parser was freed
parser_ct parser_unref(parser_ct p);

/// Sink floating reference of parser and free if no longer referenced.
///
/// \param p    parser, may be NULL
///
/// \retval p       parser is still referenced
/// \retval NULL    parser was freed
parser_ct parser_sink(parser_ct p);

/// Increase reference count and sink floating reference of parser.
///
/// \param p    parser, may be NULL
///
/// \returns    p
parser_ct parser_ref_sink(parser_ct p);

/// Get reference count of parser.
///
/// \param p    parser
///
/// \returns reference count
size_t parser_get_ref_count(parser_const_ct p);

/// Check if parser has a floating reference.
///
/// \param p    parser
///
/// \retval true    parser is floating
/// \retval false   parser is not floating
bool parser_is_floating(parser_const_ct p);

/// Get parser context.
///
/// \param p    parser
///
/// \returns parser context
void *parser_get_ctx(parser_const_ct p);

/// Free current parser context and set new one.
///
/// \param p    parser
/// \param ctx  new parser context
void parser_set_ctx(parser_ct p, const void *ctx);

/// Parse input.
///
/// \param p        parser
/// \param input    input
/// \param len      \p input length in bytes
/// \param stack    parse stack, may be NULL if no results are to be produced
///
/// \returns        number of bytes parsed
/// \retval -1/?    \todo
ssize_t parser_parse(parser_const_ct p, const void *input, size_t len, parser_stack_ct stack);

/// Push item onto parse stack.
///
/// \param stack    parse stack
/// \param type     item type
/// \param data     pointer to item data
/// \param size     item size
/// \param dtor     item destructor
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int parser_stack_push(parser_stack_ct stack, const char *type, const void *data, size_t size, parser_dtor_cb dtor);

/// Push pointer item onto parse stack.
///
/// \param stack    parse stack
/// \param type     item type
/// \param ptr      pointer item, must not be NULL
/// \param dtor     item destructor
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int parser_stack_push_p(parser_stack_ct stack, const char *type, const void *ptr, parser_dtor_cb dtor);

/// Pop item from parse stack.
///
/// \param stack    parse stack
/// \param type     item type
/// \param data     pointer to item to fill, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_PARSER_STACK_EMPTY     parse stack is empty
/// \retval -1/E_PARSER_STACK_TYPE      stack item has not requested type
int parser_stack_pop(parser_stack_ct stack, const char *type, void *data);

/// Pop argument from parse stack.
///
/// \param stack    parse stack
/// \param type     argument type
/// \param data     pointer to argument to fill, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_PARSER_ARG_MISSING     parse stack is empty
/// \retval -1/E_PARSER_ARG_TYPE        stack item has not requested type
int parser_stack_pop_arg(parser_stack_ct stack, const char *type, void *data);

/// Pop pointer item from parse stack.
///
/// \param stack    parse stack
/// \param type     item type
///
/// \returns                            pointer item
/// \retval NULL/E_PARSER_STACK_EMPTY   parse stack is empty
/// \retval NULL/E_PARSER_STACK_TYPE    stack item has not requested type
void *parser_stack_pop_p(parser_stack_ct stack, const char *type);

/// Pop pointer argument from parse stack.
///
/// \param stack    parse stack
/// \param type     argument type
///
/// \returns                            pointer item
/// \retval NULL/E_PARSER_ARG_MISSING   parse stack is empty
/// \retval NULL/E_PARSER_ARG_TYPE      stack item has not requested type
void *parser_stack_pop_arg_p(parser_stack_ct stack, const char *type);

/// Get item at position from parse stack.
///
/// \param stack    parse stack
/// \param type     item type
/// \param data     pointer to item to fill, may be NULL
/// \param pos      position of item, 0 is top of stack
///
/// \retval 0                           success
/// \retval -1/E_PARSER_STACK_EMPTY     parse stack is empty
/// \retval -1/E_PARSER_STACK_TYPE      stack item has not requested type
int parser_stack_get(parser_stack_ct stack, const char *type, void *data, size_t pos);

/// Get pointer item at position from parse stack.
///
/// \param stack    parse stack
/// \param type     item type
/// \param pos      position of item, 0 is top of stack
///
/// \returns                            pointer item
/// \retval NULL/E_PARSER_STACK_EMPTY   parse stack is empty
/// \retval NULL/E_PARSER_STACK_TYPE    stack item has not requested type
void *parser_stack_get_p(parser_stack_ct stack, const char *type, size_t pos);

/// Get item type at position from parse stack.
///
/// \param stack    parse stack
/// \param pos      position of item, 0 is top of stack
///
/// \returns                            item type
/// \retval NULL/E_PARSER_STACK_EMPTY   parse stack is empty
const char *parser_stack_get_type(parser_stack_ct stack, size_t pos);

/// Get item size at position from parse stack.
///
/// \param stack    parse stack
/// \param pos      position of item, 0 is top of stack
///
/// \returns                            item size
/// \retval -1/E_PARSER_STACK_EMPTY     parse stack is empty
ssize_t parser_stack_get_size(parser_stack_ct stack, size_t pos);

/// Drop n items from parse stack.
///
/// \param stack    parse stack
/// \param n        number of items to drop
///
/// \retval 0                           success
/// \retval -1/E_PARSER_STACK_EMPTY     less than \p n items on stack
int parser_stack_drop(parser_stack_ct stack, size_t n);

/// Get parse stack size.
///
/// \param stack    parse stack
///
/// \returns        number of items on stack
size_t parser_stack_size(parser_stack_ct stack);

/// Clear parse stack.
///
/// \param stack    parse stack
void parser_stack_clear(parser_stack_ct stack);




/*
typedef enum parser_show
{
      PARSER_SHOW_DEFAULT
    , PARSER_SHOW_TOP
    , PARSER_SHOW_GROUP
} parser_show_id;

//typedef ssize_t (*parser_parse_fold_cb)(parser_const_ct p, const char *input, size_t size, bool result, parser_stack_ct stack, void *parse_ctx);
//typedef ssize_t (*parser_parse_cb)(const char *input, size_t size, bool result, void *parser_ctx, parser_stack_ct stack, parser_parse_fold_cb parse, void *parse_ctx);
//typedef int     (*parser_show_fold_cb)(parser_const_ct p, parser_show_id mode, void *state);
//typedef int     (*parser_show_cb)(str_ct str, parser_show_id mode, void *ctx, parser_show_fold_cb show, void *state);

/// parser show callback
typedef int (*parser_show_cb)(void);



/// Set parser name.
///
/// \param p        parser
/// \param name     new parser name
void parser_set_name(parser_ct p, const char *name);

/// Set parser show callback.
///
/// \param p        parser
/// \param show     new parser show callback
void parser_set_show(parser_ct p, parser_show_cb show);

/// Define parser with name.
///
/// Set parser name.
/// Used for parser building.
/// If \p p is NULL, pass error.
///
/// \param name     parser name
/// \param p        parser
///
/// \returns                        parser
/// \retval NULL/E_PARSER_DEFINED   parser already defined
parser_ct parser_define(const char *name, parser_ct p);*/

//str_ct parser_show(parser_const_ct p);

//ssize_t parser_parse(parser_const_ct p, const char *input, size_t len, size_t n, ...); // type_id type, void *result
//ssize_t parser_parse_v(parser_const_ct p, const char *input, size_t len, size_t n, va_list ap);
//ssize_t parser_parse_x(parser_const_ct p, const char *input, size_t len, void *ctx, size_t n, ...); // type_id type, void *result
//ssize_t parser_parse_vx(parser_const_ct p, const char *input, size_t len, void *ctx, size_t n, va_list ap);



//str_ct  parser_stack_show(parser_stack_ct stack, str_ct str, size_t pos);


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
