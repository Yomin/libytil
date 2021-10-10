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

#include <ytil/parser/parser.h>
#include <ytil/def.h>
#include <ytil/def/magic.h>
#include <ytil/con/vec.h>
#include <stdlib.h>
#include <string.h>


#define MAGIC       define_magic("PAR") ///< parser magic
#define MAGIC_STACK define_magic("PST") ///< parser stack magic

/// parser
typedef struct parser
{
    DEBUG_MAGIC

    const char *name;       ///< parser name

    parser_parse_cb parse;  ///< parse callback
    parser_show_cb  show;   ///< show callback
    void            *ctx;   ///< callback context
    parser_dtor_cb  dtor;   ///< callback context dtor
} parser_st;

/// parser stack
typedef struct parser_stack
{
    DEBUG_MAGIC

    size_t  size;   ///< number of items on stack
    vec_ct  stack;  ///< stack
    vec_ct  frame;  ///< stack frames
} parser_stack_st;

/// parser stack item
typedef struct parser_stack_item
{
    const char      *type;  ///< item type
    size_t          size;   ///< item size
    parser_dtor_cb  dtor;   ///< item destructor
} parser_stack_item_st;

/*typedef struct parser_arg
{
    //type_id type;
    void *arg;
} parser_arg_st;

typedef struct parser_show_state
{
    str_ct str;
    vec_ct list;
    size_t insert;
} parser_show_st;*/

/// parser error type definition
ERROR_DEFINE_LIST(PARSER,
    ERROR_INFO(E_PARSER_FAIL, "Parser failed."),
    ERROR_INFO(E_PARSER_ABORT, "Parser aborted."),
    ERROR_INFO(E_PARSER_ERROR, "Parser aborted with error."),
    ERROR_INFO(E_PARSER_DEFINED, "Parser already defined."),
    ERROR_INFO(E_PARSER_STACK_EMPTY, "Parse stack is empty."),
    ERROR_INFO(E_PARSER_STACK_TYPE, "Wrong stack type requested."),
    ERROR_INFO(E_PARSER_ARG_MISSING, "Missing parser argument on stack."),
    ERROR_INFO(E_PARSER_ARG_TYPE, "Parser argument with wrong type on stack."),
    ERROR_INFO(E_PARSER_RESULT_MISSING, "Missing parser result on stack."),
    ERROR_INFO(E_PARSER_RESULT_TYPE, "Parser result with wrong type on stack.")
);

/// default error type for parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


parser_ct parser_new(parser_parse_cb parse, const void *ctx, parser_dtor_cb dtor)
{
    parser_ct p;

    assert(parse);

    if(!(p = calloc(1, sizeof(parser_st))))
        return error_wrap_last_errno(calloc), NULL;

    init_magic(p);

    p->parse    = parse;
    p->ctx      = (void *)ctx;
    p->dtor     = dtor;

    return p;
}

void parser_free(parser_ct p)
{
    assert_magic(p);

    if(p->dtor)
        p->dtor(p->ctx);

    free(p);
}

void parser_set_name(parser_ct p, const char *name)
{
    assert_magic(p);

    p->name = name;
}

void parser_set_show(parser_ct p, parser_show_cb show)
{
    assert_magic(p);

    p->show = show;
}

parser_ct parser_define(const char *name, parser_ct p)
{
    if(!p)
        return error_pass(), NULL;

    assert_magic(p);
    assert(name);
    return_error_if_pass(p->name, E_PARSER_DEFINED, NULL);

    p->name = name;

    return p;
}

ssize_t parser_parse(parser_const_ct p, const void *input, size_t len, parser_stack_ct stack)
{
    assert_magic(p);
    assert(input);

    return error_pass_int(p->parse(input, len, p->ctx, stack));
}

int parser_stack_push(parser_stack_ct stack, const char *type, const void *data, size_t size, parser_dtor_cb dtor)
{
    parser_stack_item_st item = {0};

    assert_magic_n(stack, MAGIC_STACK);
    assert(type);
    assert(data || !size);

    if(!stack->stack && !(stack->stack = vec_new(10 * (sizeof(parser_stack_item_st) + sizeof(void*)), 1)))
        return error_wrap(), -1;

    if(!vec_push_en(stack->stack, size, data))
        return error_wrap(), -1;

    item.type   = type;
    item.size   = size;
    item.dtor   = dtor;

    if(!vec_push_en(stack->stack, sizeof(parser_stack_item_st), &item))
        return error_wrap(), vec_pop_n(stack->stack, size), -1;

    stack->size++;

    return 0;
}

int parser_stack_push_p(parser_stack_ct stack, const char *type, const void *ptr, parser_dtor_cb dtor)
{
    return error_pass_int(parser_stack_push(stack, type, &ptr, sizeof(void *), dtor));
}

int parser_stack_pop(parser_stack_ct stack, const char *type, void *data)
{
    parser_stack_item_st item;

    assert_magic_n(stack, MAGIC_STACK);
    assert(type);
    return_error_if_fail(stack->size, E_PARSER_STACK_EMPTY, -1);

    vec_get_n(stack->stack, &item, -sizeof(parser_stack_item_st), sizeof(parser_stack_item_st));

    if(strcmp(type, item.type))
        return error_set(E_PARSER_STACK_TYPE), -1;

    vec_pop_n(stack->stack, sizeof(parser_stack_item_st));
    vec_pop_en(stack->stack, data, item.size);
    stack->size--;

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
static size_t parser_stack_get_offset(parser_stack_ct stack, size_t pos)
{
    parser_stack_item_st *item;
    size_t offset;

    for(offset = 0; pos; pos--)
    {
        item    = vec_at(stack->stack, -offset - sizeof(parser_stack_item_st));
        offset  += sizeof(parser_stack_item_st) + item->size;
    }

    return offset;
}

int parser_stack_get(parser_stack_ct stack, const char *type, void *data, size_t pos)
{
    parser_stack_item_st *item;
    size_t offset;

    assert_magic_n(stack, MAGIC_STACK);
    assert(type);
    assert(data);
    return_error_if_fail(pos < stack->size, E_PARSER_STACK_EMPTY, -1);

    offset  = parser_stack_get_offset(stack, pos);
    item    = vec_at(stack->stack, -offset - sizeof(parser_stack_item_st));

    if(strcmp(type, item->type))
        return error_set(E_PARSER_STACK_TYPE), -1;

    offset += sizeof(parser_stack_item_st) + item->size;
    vec_get_n(stack->stack, data, -offset, item->size);

    return 0;
}

void *parser_stack_get_p(parser_stack_ct stack, const char *type, size_t pos)
{
    void *data;

    if(parser_stack_get(stack, type, &data, pos))
        return error_pass(), NULL;

    return data;
}

const char *parser_stack_get_type(parser_stack_ct stack, size_t pos)
{
    parser_stack_item_st *item;
    size_t offset;

    assert_magic_n(stack, MAGIC_STACK);
    return_error_if_fail(pos < stack->size, E_PARSER_STACK_EMPTY, NULL);

    offset  = parser_stack_get_offset(stack, pos);
    item    = vec_at(stack->stack, -offset - sizeof(parser_stack_item_st));

    return item->type;
}

ssize_t parser_stack_get_size(parser_stack_ct stack, size_t pos)
{
    parser_stack_item_st *item;
    size_t offset;

    assert_magic_n(stack, MAGIC_STACK);
    return_error_if_fail(pos < stack->size, E_PARSER_STACK_EMPTY, -1);

    offset  = parser_stack_get_offset(stack, pos);
    item    = vec_at(stack->stack, -offset - sizeof(parser_stack_item_st));

    return item->size;
}

int parser_stack_drop(parser_stack_ct stack, size_t n)
{
    parser_stack_item_st item;

    assert_magic_n(stack, MAGIC_STACK);
    return_error_if_fail(n <= stack->size, E_PARSER_STACK_EMPTY, -1);

    for(; n; n--)
    {
        vec_pop_en(stack->stack, &item, sizeof(parser_stack_item_st));

        if(item.dtor)
            item.dtor(vec_at(stack->stack, -item.size));

        vec_pop_n(stack->stack, item.size);
        stack->size--;
    }

    return 0;
}

size_t parser_stack_size(parser_stack_ct stack)
{
    assert_magic_n(stack, MAGIC_STACK);

    return stack->size;
}

void parser_stack_clear(parser_stack_ct stack)
{
    assert_magic_n(stack, MAGIC_STACK);

    parser_stack_drop(stack, stack->size);
}


/*
static bool vec_cmp_parser(vec_const_ct vec, const void *elem, void *ctx)
{
    parser_const_ct p = *(parser_const_ct*)elem;
    
    return !str_cmp(p->name, ctx);
}

static int parser_show_fold(parser_const_ct p, parser_show_id mode, void *ctx)
{
    show_st *state = ctx;
    
    assert_magic(p, NULL);
    assert_fail(ctx, EINVAL, NULL);
    
    if(p->name)
    {
        if(!vec_find(state->list, vec_cmp_parser, (void*)p->name)
        && !vec_insert_p(state->list, state->insert++, p))
            return -1;
        
        return str_append_f(state->str, "<%s>", str_c(p->name)) ? 0 : -1;
    }
    
    if(p->show)
        return p->show(state->str, mode, p->ctx, parser_show_fold, state);
    else
        return str_append_l(state->str, "unknown") ? 0 : -1;
}

str_ct parser_show(parser_const_ct p)
{
    show_st state = { .insert = 1 };
    parser_const_ct next;
    str_const_ct name, unnamed = LSTR("<unnamed>");
    size_t i;
    int rc = 0;
    
    if(!check_magic(p))
        return str_new_l("<invalid parser>");
    
    if(!(state.str = str_new_l("")))
        return NULL;
    
    if(!(state.list = vec_new(10, sizeof(parser_const_ct))))
        return str_unref(state.str), NULL;
    
    vec_push_e(state.list, &p);
    
    for(i=0; !rc && (next = vec_at_p(state.list, i)); i++, state.insert = i+1)
    {
        name = next->name ? next->name : unnamed;
        
        if(!str_append_f(state.str, "%s%s: ", str_c(name),
                str_len(name) < 8 ? "\t\t" : str_len(name) < 16 ? "\t" : ""))
            rc = -1;
        
        if(!rc && next->show)
            rc = next->show(state.str, PARSER_SHOW_TOP, next->ctx, parser_show_fold, &state);
        else if(!rc && !str_append_l(state.str, "unknown"))
            rc = -1;
        
        if(!rc && !str_append_l(state.str, "\n"))
            rc = -1;
    }
    
    vec_free(state.list);
    
    if(rc)
        return str_unref(state.str), NULL;
    
    return state.str;
}

static ssize_t parser_parse_fold(parser_const_ct p, const char *input, size_t len, bool result, parser_stack_ct stack, void *parse_ctx)
{
    unsigned short size;
    ssize_t rc;
    
    if((!stack->frame && !(stack->frame = vec_new(10, sizeof(unsigned short))))
    || !vec_push_e(stack->frame, &stack->size))
        return PARSER_ABORT;
    
    stack->size = 0;
    
    if((rc = p->parse(input, len, result, p->ctx, stack, parser_parse_fold, parse_ctx)) < 0)
        parser_stack_clear(stack);
    else if(!result && stack->size)
    {
        assert(!result && stack->size);
        parser_stack_clear(stack);
    }
    
    vec_pop_e(stack->frame, &size);
    stack->size += size;
    
    return rc;
}

ssize_t parser_parse(parser_const_ct p, const char *input, size_t len, size_t n, ...)
{
    va_list ap;
    ssize_t count;
    
    va_start(ap, n);
    count = parser_parse_vx(p, input, len, NULL, n, ap);
    va_end(ap);
    
    return count;
}

ssize_t parser_parse_v(parser_const_ct p, const char *input, size_t len, size_t n, va_list ap)
{
    return parser_parse_vx(p, input, len, NULL, n, ap);
}

ssize_t parser_parse_x(parser_const_ct p, const char *input, size_t len, void *ctx, size_t n, ...)
{
    va_list ap;
    ssize_t count;
    
    va_start(ap, n);
    count = parser_parse_vx(p, input, len, ctx, n, ap);
    va_end(ap);
    
    return count;
}

ssize_t parser_parse_vx(parser_const_ct p, const char *input, size_t len, void *ctx, size_t n, va_list ap)
{
    parser_stack_st stack = { 0 };
    parser_arg_st *results;
    ssize_t rc;
    
    assert_magic(p);
    assert(input);
    
    init_magic_n(&stack, MAGIC_STACK);
    
    if((rc = p->parse(input, len, !!n, p->ctx, &stack, parser_parse_fold, ctx)) < 0)
        ;
    else if(n > stack.size)
    {
        assert(n <= stack.size);
        
        errno = EINVAL;
        rc = PARSER_ABORT;
    }
    else
    {
        parser_stack_drop(&stack, stack.size - n);
        
        results = alloca(n*sizeof(arg_st));
        
        for(n=0; n < stack.size; n++)
        {
            results[n].type = va_arg(ap, type_id);
            results[n].arg = va_arg(ap, void*);
        }
        
        for(; n; n--)
            if(results[n-1].type)
                parser_stack_pop(&stack, results[n-1].type, results[n-1].arg);
            else
                parser_stack_drop(&stack, 1);
    }
    
    if(stack.stack)
    {
        parser_stack_clear(&stack);
        vec_free(stack.stack);
    }
    
    if(stack.frame)
        vec_free(stack.frame);
    
    return rc;
}

str_ct parser_stack_show(parser_stack_ct stack, str_ct str, size_t pos)
{
    ssize_t offset;
    type_id type;
    void *value;
    
    assert_nmagic(stack, MAGIC_STACK, NULL);
    
    if((offset = get_offset(stack, pos)) < 0)
        return TYPE_INVALID;
    
    type = *(type_id*)vec_at(stack->stack, -offset - sizeof(type_id));
    value = vec_at(stack->stack, -offset - sizeof(type_id) - type_size(type));
    
    return type_show(str, type, value);
}
*/
