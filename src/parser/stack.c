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

#include <ytil/parser/stack.h>
#include <ytil/def.h>
#include <ytil/def/magic.h>
#include <ytil/con/vec.h>
#include <stdlib.h>
#include <string.h>


#define MAGIC define_magic("PST") ///< parse stack magic

/// parse stack item
typedef struct parser_stack_item
{
    const char              *type;  ///< item type, NULL if already removed
    void                    *data;  ///< item data
    size_t                  size;   ///< item size
    parser_stack_dtor_cb    dtor;   ///< item destructor
} parser_stack_item_st;

/// parse stack frame
typedef struct parser_stack_frame
{
    size_t  size;       ///< number of items
    bool    active;     ///< whether items can be pushed
} parser_stack_frame_st;
/// parse stack

typedef struct parser_stack
{
    DEBUG_MAGIC

    parser_stack_frame_st   frame;  ///< current stack frame
    vec_ct                  items;  ///< stack item info
    vec_ct                  data;   ///< stack item data
    vec_ct                  frames; ///< stack frames
} parser_stack_st;



/// default error type for parse stack module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


parser_stack_ct parser_stack_new(void)
{
    parser_stack_ct stack;

    if(!(stack = calloc(1, sizeof(parser_stack_st))))
        return error_wrap_last_errno(calloc), NULL;

    init_magic(stack);

    stack->frame.active = true;

    return stack;
}

/// Vector dtor callback for freeing stack item.
///
/// \implements vec_dtor_cb
static void parser_stack_vec_free_item(vec_const_ct vec, void *elem, void *ctx)
{
    parser_stack_item_st *item = elem;

    if(item->type && item->dtor)
        item->dtor(item->data);
}

void parser_stack_free(parser_stack_ct stack)
{
    assert_magic(stack);

    if(stack->items)
        vec_free_f(stack->items, parser_stack_vec_free_item, NULL);

    if(stack->data)
        vec_free(stack->data);

    if(stack->frames)
        vec_free(stack->frames);

    free(stack);
}

int parser_stack_frame_push(parser_stack_ct stack)
{
    assert_magic(stack);

    if(!stack->frames && !(stack->frames = vec_new(sizeof(parser_stack_frame_st))))
        return error_wrap(), -1;

    if(!vec_push_e(stack->frames, &stack->frame))
        return error_wrap(), -1;

    stack->frame.size = 0;

    return 0;
}

int parser_stack_frame_pop(parser_stack_ct stack)
{
    parser_stack_frame_st frame;

    assert_magic(stack);

    if(!stack->frames || vec_pop_e(stack->frames, &frame))
        return error_set(E_PARSER_STACK_MISSING), -1;

    stack->frame.size += frame.size;

    return 0;
}

size_t parser_stack_frame_depth(parser_stack_ct stack)
{
    assert_magic(stack);

    return stack->frames ? vec_size(stack->frames) : 0;
}

int parser_stack_push(parser_stack_ct stack, const char *type, const void *data, size_t size, parser_stack_dtor_cb dtor)
{
    parser_stack_item_st *item;

    assert_magic(stack);
    assert(type);
    assert(data || !size);

    if(!stack->frame.active)
        return 0;

    if(!stack->items && !(stack->items = vec_new(sizeof(parser_stack_item_st))))
        return error_wrap(), -1;

    if(!stack->data && !(stack->data = vec_new_c(10 * sizeof(void *), 1)))
        return error_wrap(), -1;

    if(!(item = vec_push(stack->items)))
        return error_wrap(), -1;

    if(data && !(item->data = vec_push_en(stack->data, size, data)))
        return error_wrap(), vec_pop(stack->items), -1;

    item->type  = type;
    item->size  = size;
    item->dtor  = dtor;

    stack->frame.size++;

    return 0;
}

int parser_stack_push_p(parser_stack_ct stack, const char *type, const void *ptr, parser_stack_dtor_cb dtor)
{
    return error_pass_int(parser_stack_push(stack, type, &ptr, sizeof(void *), dtor));
}

int parser_stack_pop(parser_stack_ct stack, const char *type, void *data)
{
    parser_stack_item_st *item;

    assert_magic(stack);
    assert(type);
    return_error_if_fail(stack->frame.size, E_PARSER_STACK_MISSING, -1);

    item = vec_at(stack->items, -1);

    // drop already removed items
    while(!item->type)
    {
        vec_pop_en(stack->data, NULL, item->size);
        vec_pop(stack->items);

        item = vec_at(stack->items, -1);
    }

    return_error_if_fail(!strcmp(type, item->type), E_PARSER_STACK_TYPE, -1);

    if(!data && item->dtor)
        item->dtor(item->data);

    vec_pop_en(stack->data, data, item->size);
    vec_pop(stack->items);

    stack->frame.size--;

    return 0;
}

void *parser_stack_pop_p(parser_stack_ct stack, const char *type)
{
    void *data;

    if(parser_stack_pop(stack, type, &data))
        return error_pass(), NULL;

    if(!data)
        return error_set(E_PARSER_STACK_NULL), NULL;

    return data;
}

int parser_stack_pop_arg(parser_stack_ct stack, const char *type, void *data)
{
    parser_stack_frame_st *frame;
    parser_stack_item_st *item;
    size_t n;

    assert_magic(stack);
    assert(type);

    if(!stack->frames || !(frame = vec_at(stack->frames, -1)) || !frame->size)
        return error_set(E_PARSER_STACK_MISSING), -1;

    // skip already removed items
    for(n = 0;; n++)
    {
        item = vec_at(stack->items, -(stack->frame.size + n + 1));

        if(item->type)
            break;
    }

    return_error_if_fail(!strcmp(type, item->type), E_PARSER_STACK_TYPE, -1);

    if(data)
        memcpy(data, item->data, item->size);
    else if(item->dtor)
        item->dtor(item->data);

    if(!stack->frame.size)
    {
        vec_pop_en(stack->data, NULL, item->size);
        vec_pop(stack->items);
    }
    else
    {
        item->type = NULL; // only mark as removed
    }

    frame->size--;

    return 0;
}

void *parser_stack_pop_arg_p(parser_stack_ct stack, const char *type)
{
    void *data;

    if(parser_stack_pop_arg(stack, type, &data))
        return error_pass(), NULL;

    return data;
}

void *parser_stack_at(parser_stack_const_ct stack, const char *type, size_t pos)
{
    parser_stack_item_st *item;

    assert_magic(stack);
    assert(type);
    return_error_if_fail(pos < stack->frame.size, E_PARSER_STACK_MISSING, NULL);

    item = vec_at(stack->items, -(pos + 1));

    return_error_if_fail(!strcmp(type, item->type), E_PARSER_STACK_TYPE, NULL);

    return item->data;
}

const char *parser_stack_get_type(parser_stack_const_ct stack, size_t pos)
{
    parser_stack_item_st *item;

    assert_magic(stack);
    return_error_if_fail(pos < stack->frame.size, E_PARSER_STACK_MISSING, NULL);

    item = vec_at(stack->items, -(pos + 1));

    return item->type;
}

ssize_t parser_stack_get_size(parser_stack_const_ct stack, size_t pos)
{
    parser_stack_item_st *item;

    assert_magic(stack);
    return_error_if_fail(pos < stack->frame.size, E_PARSER_STACK_MISSING, -1);

    item = vec_at(stack->items, -(pos + 1));

    return item->size;
}

void parser_stack_activate(parser_stack_ct stack, bool active)
{
    assert_magic(stack);

    stack->frame.active = active;
}

bool parser_stack_is_active(parser_stack_const_ct stack)
{
    assert_magic(stack);

    return stack->frame.active;
}

int parser_stack_drop(parser_stack_ct stack, size_t n)
{
    parser_stack_item_st item;

    assert_magic(stack);
    return_error_if_fail(n <= stack->frame.size, E_PARSER_STACK_MISSING, -1);

    while(n)
    {
        vec_pop_e(stack->items, &item);

        if(item.type)
        {
            if(item.dtor)
                item.dtor(item.data);

            stack->frame.size--;
            n--;
        }

        vec_pop_n(stack->data, item.size);
    }

    return 0;
}

void parser_stack_clear(parser_stack_ct stack)
{
    assert_magic(stack);

    parser_stack_drop(stack, stack->frame.size);
}

size_t parser_stack_size(parser_stack_const_ct stack)
{
    assert_magic(stack);

    return stack->frame.size;
}

ssize_t parser_stack_args(parser_stack_const_ct stack)
{
    parser_stack_frame_st *frame;

    assert_magic(stack);

    if(!stack->frames || !(frame = vec_at(stack->frames, -1)))
        return error_set(E_PARSER_STACK_MISSING), -1;

    return frame->size;
}