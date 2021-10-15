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

#ifndef YTIL_PARSER_STACK_H_INCLUDED
#define YTIL_PARSER_STACK_H_INCLUDED

#include <ytil/gen/error.h>
#include <sys/types.h>


/// parser stack item dtor callback
///
/// \param item     item to destroy
typedef void (*parser_stack_dtor_cb)(void *item);

typedef       struct parser_stack *parser_stack_ct;         ///< parser stack type
typedef const struct parser_stack *parser_stack_const_ct;   ///< const parser stack type


#include <ytil/parser/parser.h>


/// New parse stack.
///
/// \returns                    parse stack
/// \retval NULL/E_GENERIC_OOM  out of memory
parser_stack_ct parser_stack_new(void);

/// Free parse stack.
///
/// \param stack    parse stack
void parser_stack_free(parser_stack_ct stack);

/// Push new stack frame.
///
/// A new stack frame sets the stack size to 0.
/// This means all current stack items are hidden
/// and only accessible with parser_stack_pop_arg().
///
/// \param stack    parse stack
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int parser_stack_frame_push(parser_stack_ct stack);

/// Pop current stack frame.
///
/// The current stack frame is merged with the previous one.
///
/// \param stack    parse stack
///
/// \retval 0                           success
/// \retval -1/E_PARSER_STACK_MISSING   no frame left
int parser_stack_frame_pop(parser_stack_ct stack);

/// Get number of stack frames.
///
/// \param stack    parse stack
///
/// \returns        number of stack frames
size_t parser_stack_frame_depth(parser_stack_ct stack);

/// Push item onto current parse stack frame.
///
/// \param stack    parse stack
/// \param type     item type
/// \param data     pointer to item data
/// \param size     item size
/// \param dtor     item destructor
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int parser_stack_push(parser_stack_ct stack, const char *type, const void *data, size_t size, parser_stack_dtor_cb dtor);

/// Push pointer item onto current parse stack frame.
///
/// \param stack    parse stack
/// \param type     item type
/// \param ptr      pointer item
/// \param dtor     item destructor
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int parser_stack_push_p(parser_stack_ct stack, const char *type, const void *ptr, parser_stack_dtor_cb dtor);

/// Pop item from current parse stack frame.
///
/// \param stack    parse stack
/// \param type     item type
/// \param data     pointer to item to fill, if NULL item is destructed
///
/// \retval 0                           success
/// \retval -1/E_PARSER_STACK_MISSING   parse stack frame is empty
/// \retval -1/E_PARSER_STACK_TYPE      stack item has not requested type
int parser_stack_pop(parser_stack_ct stack, const char *type, void *data);

/// Pop pointer item from current parse stack frame.
///
/// \param stack    parse stack
/// \param type     item type
///
/// \returns                                pointer item
/// \retval NULL/E_PARSER_STACK_MISSING     parse stack frame is empty
/// \retval NULL/E_PARSER_STACK_TYPE        stack item has not requested type
/// \retval NULL/E_PARSER_STACK_NULL        NULL pointer is valid
void *parser_stack_pop_p(parser_stack_ct stack, const char *type);

/// Pop item from last parse stack frame.
///
/// \param stack    parse stack
/// \param type     item type
/// \param data     pointer to item to fill, if NULL item is destructed
///
/// \retval 0                           success
/// \retval -1/E_PARSER_ARG_MISSING     last parse stack frame is empty
/// \retval -1/E_PARSER_ARG_TYPE        stack item has not requested type
int parser_stack_pop_arg(parser_stack_ct stack, const char *type, void *data);

/// Pop pointer item from last parse stack frame.
///
/// \param stack    parse stack
/// \param type     item type
///
/// \returns                            pointer item
/// \retval NULL/E_PARSER_ARG_MISSING   last parse stack is empty
/// \retval NULL/E_PARSER_ARG_TYPE      stack item has not requested type
void *parser_stack_pop_arg_p(parser_stack_ct stack, const char *type);

/// Get item at position from current parse stack frame.
///
/// \param stack    parse stack
/// \param type     item type
/// \param pos      position of item, 0 is top of stack
///
/// \returns                                pointer to stack item
/// \retval NULL/E_PARSER_STACK_MISSING     requested item not in stack frame
/// \retval NULL/E_PARSER_STACK_TYPE        stack item has not requested type
void *parser_stack_at(parser_stack_const_ct stack, const char *type, size_t pos);

/// Get item type at position from current parse stack frame.
///
/// \param stack    parse stack
/// \param pos      position of item, 0 is top of stack
///
/// \returns                                item type
/// \retval NULL/E_PARSER_STACK_MISSING     requested item not in stack frame
const char *parser_stack_get_type(parser_stack_const_ct stack, size_t pos);

/// Get item size at position from current parse stack frame.
///
/// \param stack    parse stack
/// \param pos      position of item, 0 is top of stack
///
/// \returns                                item size
/// \retval -1/E_PARSER_STACK_MISSING       requested item not in stack frame
ssize_t parser_stack_get_size(parser_stack_const_ct stack, size_t pos);

/// Enable/Disable result pushing in the current stack frame.
///
/// \param stack    parse stack
/// \param active   whether results can be pushed
void parser_stack_activate(parser_stack_ct stack, bool active);

/// Check whether results can be pushed in the current stack frame.
///
/// \param stack    parse stack
///
/// \retval true    results can be pushed
/// \retval false   results can not be pushed
bool parser_stack_is_active(parser_stack_const_ct stack);

/// Drop n items from current parse stack frame.
///
/// \param stack    parse stack
/// \param n        number of items to drop
///
/// \retval 0                           success
/// \retval -1/E_PARSER_STACK_MISSING   less than \p n items on stack
int parser_stack_drop(parser_stack_ct stack, size_t n);

/// Clear current parse stack frame.
///
/// \param stack    parse stack
void parser_stack_clear(parser_stack_ct stack);

/// Get current parse stack frame size.
///
/// \param stack    parse stack
///
/// \returns        number of items on stack
size_t parser_stack_size(parser_stack_const_ct stack);

/// Get last parse stack frame size.
///
/// \param stack    parse stack
///
/// \returns                            number of items on stack
/// \retval -1/E_PARSER_STACK_MISSING   no last frame available
ssize_t parser_stack_args(parser_stack_const_ct stack);


#endif
