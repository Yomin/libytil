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
    const char              *type;  ///< item type
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
    vec_ct                  items;  ///< stack items
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

void parser_stack_free(parser_stack_ct stack)
{
    parser_stack_item_st item;

    assert_magic(stack);

    if(stack->items)
    {
        while(!vec_pop_en(stack->items, &item, sizeof(parser_stack_item_st)))
        {
            if(item.dtor)
                item.dtor(vec_at(stack->items, -item.size));

            vec_pop_n(stack->items, item.size);
        }

        vec_free(stack->items);
    }

    if(stack->frames)
        vec_free(stack->frames);

    free(stack);
}

/// \todo remove
#include <stdio.h>

int parser_stack_frame_push(parser_stack_ct stack)
{
    assert_magic(stack);

    if(!stack->frames && !(stack->frames = vec_new(10, sizeof(parser_stack_frame_st))))
        return error_wrap(), -1;

    if(!vec_push_e(stack->frames, &stack->frame))
        return error_wrap(), -1;

    stack->frame.size = 0;

    //printf("%02zu push frame\n", vec_size(stack->frames) - 1);

    return 0;
}

int parser_stack_frame_pop(parser_stack_ct stack)
{
    size_t size;

    assert_magic(stack);

    size = stack->frame.size;

    if(!stack->frames || vec_pop_e(stack->frames, &stack->frame))
        return error_set(E_PARSER_STACK_EMPTY), -1;

    stack->frame.size += size;

    /*printf("%02zu pop frame, items +%zu = %zu, results %s\n",
        vec_size(stack->frames) + 1, size, stack->frame.size,
        stack->frame.active ? "active" : "inactive");*/

    return 0;
}

/// foo
static int print(parser_stack_const_ct stack, const char *action, const char *type, const void *data)
{
    size_t frame = stack->frames ? vec_size(stack->frames) : 0;

    if(!strcmp(type, "char"))
        printf("%02zu %s char '%c'\n", frame, action, ((char*)data)[0]);
    else if(!strcmp(type, "string"))
        printf("%02zu %s string '%s'\n", frame, action, *(char**)data);
    else if(!strcmp(type, "int"))
        printf("%02zu %s int '%d'\n", frame, action, *(int*)data);
    else if(!strcmp(type, "uint"))
        printf("%02zu %s uint '%u'\n", frame, action, *(unsigned int*)data);
    else
        printf("%02zu %s %s\n", frame, action, type);

    return 0;
}

int parser_stack_push(parser_stack_ct stack, const char *type, const void *data, size_t size, parser_stack_dtor_cb dtor)
{
    parser_stack_item_st item = {0};

    assert_magic(stack);
    assert(type);
    assert(data || !size);

    if(!stack->frame.active)
        return 0;

    if(!stack->items && !(stack->items = vec_new(10 * (sizeof(parser_stack_item_st) + sizeof(void*)), 1)))
        return error_wrap(), -1;

    if(!vec_push_en(stack->items, size, data))
        return error_wrap(), -1;

    item.type   = type;
    item.size   = size;
    item.dtor   = dtor;

    if(!vec_push_en(stack->items, sizeof(parser_stack_item_st), &item))
        return error_wrap(), vec_pop_n(stack->items, size), -1;

    stack->frame.size++;

    print(stack, "push", type, data);

    return 0;
}

int parser_stack_push_p(parser_stack_ct stack, const char *type, const void *ptr, parser_stack_dtor_cb dtor)
{
    return error_pass_int(parser_stack_push(stack, type, &ptr, sizeof(void *), dtor));
}

int parser_stack_pop(parser_stack_ct stack, const char *type, void *data)
{
    parser_stack_item_st item;

    assert_magic(stack);
    assert(type);
    return_error_if_fail(stack->frame.size, E_PARSER_STACK_EMPTY, -1);

    vec_get_n(stack->items, &item, -sizeof(parser_stack_item_st), sizeof(parser_stack_item_st));

    if(strcmp(type, item.type))
        return error_set(E_PARSER_STACK_TYPE), -1;

    vec_pop_n(stack->items, sizeof(parser_stack_item_st));
    vec_pop_en(stack->items, data, item.size);
    stack->frame.size--;

    print(stack, "pop", type, data);

    return 0;
}

int parser_stack_pop_arg(parser_stack_ct stack, const char *type, void *data)
{
    if(!parser_stack_pop(stack, type, data))
        return 0;

    switch(error_code(0))
    {
    case E_PARSER_STACK_EMPTY:
        return error_push(E_PARSER_ARG_MISSING), -1;

    case E_PARSER_STACK_TYPE:
        return error_push(E_PARSER_ARG_TYPE), -1;

    default:
        return error_pass(), -1;
    }
}

void *parser_stack_pop_p(parser_stack_ct stack, const char *type)
{
    void *data;

    if(parser_stack_pop(stack, type, &data))
        return error_pass(), NULL;

    return data;
}

void *parser_stack_pop_arg_p(parser_stack_ct stack, const char *type)
{
    void *data;

    if(parser_stack_pop_arg(stack, type, &data))
        return error_pass(), NULL;

    return data;
}

/// Get byte offset of item down the stack.
///
/// \param stack    stack
/// \param pos      position from top of stack
///
/// \returns byte offset, 0 for position 0
static size_t parser_stack_get_offset(parser_stack_const_ct stack, size_t pos)
{
    parser_stack_item_st *item;
    size_t offset;

    for(offset = 0; pos; pos--)
    {
        item    = vec_at(stack->items, -offset - sizeof(parser_stack_item_st));
        offset  += sizeof(parser_stack_item_st) + item->size;
    }

    return offset;
}

int parser_stack_get(parser_stack_const_ct stack, const char *type, void *data, size_t pos)
{
    parser_stack_item_st *item;
    size_t offset;

    assert_magic(stack);
    assert(type);
    assert(data);
    return_error_if_fail(pos < stack->frame.size, E_PARSER_STACK_EMPTY, -1);

    offset  = parser_stack_get_offset(stack, pos);
    item    = vec_at(stack->items, -offset - sizeof(parser_stack_item_st));

    if(strcmp(type, item->type))
        return error_set(E_PARSER_STACK_TYPE), -1;

    offset += sizeof(parser_stack_item_st) + item->size;
    vec_get_n(stack->items, data, -offset, item->size);

    print(stack, "get", type, data);

    return 0;
}

void *parser_stack_get_p(parser_stack_const_ct stack, const char *type, size_t pos)
{
    void *data;

    if(parser_stack_get(stack, type, &data, pos))
        return error_pass(), NULL;

    return data;
}

const char *parser_stack_get_type(parser_stack_const_ct stack, size_t pos)
{
    parser_stack_item_st *item;
    size_t offset;

    assert_magic(stack);
    return_error_if_fail(pos < stack->frame.size, E_PARSER_STACK_EMPTY, NULL);

    offset  = parser_stack_get_offset(stack, pos);
    item    = vec_at(stack->items, -offset - sizeof(parser_stack_item_st));

    return item->type;
}

ssize_t parser_stack_get_size(parser_stack_const_ct stack, size_t pos)
{
    parser_stack_item_st *item;
    size_t offset;

    assert_magic(stack);
    return_error_if_fail(pos < stack->frame.size, E_PARSER_STACK_EMPTY, -1);

    offset  = parser_stack_get_offset(stack, pos);
    item    = vec_at(stack->items, -offset - sizeof(parser_stack_item_st));

    return item->size;
}

void parser_stack_activate(parser_stack_ct stack, bool active)
{
    assert_magic(stack);

    stack->frame.active = active;

    /*printf("%02zu results %s\n",
        stack->frames ? vec_size(stack->frames) : 0,
        active ? "activated" : "deactivated");*/
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
    return_error_if_fail(n <= stack->frame.size, E_PARSER_STACK_EMPTY, -1);

    for(; n; n--)
    {
        vec_pop_en(stack->items, &item, sizeof(parser_stack_item_st));

        print(stack, "drop", item.type, vec_at(stack->items, -item.size));

        if(item.dtor)
            item.dtor(vec_at(stack->items, -item.size));

        vec_pop_n(stack->items, item.size);
        stack->frame.size--;
    }

    return 0;
}

size_t parser_stack_size(parser_stack_const_ct stack)
{
    assert_magic(stack);

    return stack->frame.size;
}

void parser_stack_clear(parser_stack_ct stack)
{
    assert_magic(stack);

    parser_stack_drop(stack, stack->frame.size);
}
