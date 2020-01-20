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

#include <ytil/parser/parser.h>
#include <ytil/def.h>
#include <ytil/def/magic.h>
#include <ytil/con/vec.h>
//#include <ytil/type/parser.h>

#include <stdlib.h>
#include <string.h>


#define MAGIC       define_magic("PAR")
#define MAGIC_STACK define_magic("PST")

typedef struct parser
{
    DEBUG_MAGIC
    
    str_const_ct name;
    
    parser_parse_cb parse;
    parser_show_cb show;
    void *ctx;
    parser_dtor_cb dtor;
} parser_st;

typedef struct parser_stack
{
    DEBUG_MAGIC
    
    unsigned short size;
    vec_ct stack, frame;
} parser_stack_st;

typedef struct parser_arg
{
    //type_id type;
    void *arg;
} parser_arg_st;

typedef struct parser_show_state
{
    str_ct str;
    vec_ct list;
    size_t insert;
} parser_show_st;

/// parser error type definition
ERROR_DEFINE_LIST(PARSER,
    ERROR_INFO(E_PARSER_INVALID_NAME, "Invalid parser name.")
);

/// default error type for parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


parser_ct parser_new(parser_parse_cb parse, parser_show_cb show, void *ctx, parser_dtor_cb dtor)
{
    parser_ct p;
    
    assert(parse);
    
    if(!(p = calloc(1, sizeof(parser_st))))
        return error_wrap_last_errno(calloc), NULL;
    
    init_magic(p);
    
    p->parse = parse;
    p->show = show;
    p->ctx = ctx;
    p->dtor = dtor;
    
    return p;
}

void parser_free(parser_ct p)
{
    assert_magic(p);
    
    if(p->dtor)
        p->dtor(p->ctx);
    
    if(p->name)
        str_unref(p->name);
    
    free(p);
}

parser_ct parser_set_name(parser_ct p, str_const_ct name)
{
    assert_magic(p);
    return_error_if_pass(str_is_empty(name), E_PARSER_INVALID_NAME, NULL);
    
    if(!(name = str_ref(name)))
        return error_wrap(), NULL;
    
    if(p->name)
        str_unref(p->name);
    
    p->name = name;
    
    return p;
}

parser_ct parser_set_show(parser_ct p, parser_show_cb show)
{
    assert_magic(p);
    assert(show);
    
    p->show = show;
    
    return p;
}

parser_ct parser_define(str_const_ct name, parser_ct p)
{
    if(!p)
        return error_pass(), NULL;
    
    assert_magic(p);
    assert(!p->name);
    
    if(!parser_set_name(p, name))
        return error_pass(), parser_free(p), NULL;
    
    return p;
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

int parser_stack_push(parser_stack_ct stack, type_id type, const void *result)
{
    size_t size;
    
    assert_nmagic(stack, MAGIC_STACK, -1);
    assert_fail(result && type, EINVAL, -1);
    
    if(!stack->stack
    && !(stack->stack = vec_new(10*(sizeof(type_id)+sizeof(void*)), sizeof(char))))
        return -1;
    
    size = type_size(type);
    
    if(!vec_push_en(stack->stack, size, result))
        return -1;
    
    if(!vec_push_en(stack->stack, sizeof(type_id), &type))
        return vec_pop_n(stack->stack, size), -1;
    
    stack->size++;
    
    return 0;
}

int parser_stack_push_p(parser_stack_ct stack, type_id type, const void *result)
{
    assert_fail(type_size(type) == sizeof(void*), EINVAL, -1);
    
    return parser_stack_push(stack, type, &result);
}

int parser_stack_pop(parser_stack_ct stack, type_id type, void *result)
{
    type_id stack_type;
    
    assert_nmagic(stack, MAGIC_STACK, -1);
    
    if(!stack->size)
        return errno = ENOENT, -1;
    
    vec_pop_en(stack->stack, sizeof(type_id), &stack_type);
    
    if(type != stack_type)
    {
        vec_push_en(stack->stack, sizeof(type_id), &stack_type);
        return errno = ENOMSG, -1;
    }
    
    vec_pop_en(stack->stack, type_size(type), result);
    stack->size--;
    
    return 0;
}

void *parser_stack_pop_p(parser_stack_ct stack, type_id type)
{
    void *result = NULL;
    
    parser_stack_pop(stack, type, &result);
    
    return result;
}

static ssize_t get_offset(parser_stack_ct stack, size_t pos)
{
    type_id type;
    ssize_t offset;
    
    if(pos >= stack->size)
        return errno = ENOENT, -1;
    
    for(offset=0; pos; pos--, offset += sizeof(type_id) + type_size(type))
        type = *(type_id*)vec_at(stack->stack, -offset - sizeof(type_id));
    
    return offset;
}

int parser_stack_get(parser_stack_ct stack, size_t pos, type_id type, void *result)
{
    type_id stack_type;
    ssize_t offset;
    
    assert_nmagic(stack, MAGIC_STACK, -1);
    assert_fail(result, EINVAL, -1);
    
    if((offset = get_offset(stack, pos)) < 0)
        return -1;
    
    stack_type = *(type_id*)vec_at(stack->stack, -offset - sizeof(type_id));
    offset += sizeof(type_id) + type_size(stack_type);
    
    if(type != stack_type)
        return errno = ENOMSG, -1;
    
    vec_get_n(stack->stack, -offset, type_size(type), result);
    
    return 0;
}

void *parser_stack_get_p(parser_stack_ct stack, size_t pos, type_id type)
{
    void *result = NULL;
    
    parser_stack_get(stack, pos, type, &result);
    
    return result;
}

int parser_stack_drop(parser_stack_ct stack, size_t n)
{
    type_id type;
    size_t size;
    
    assert_nmagic(stack, MAGIC_STACK, -1);
    assert_fail(n <= stack->size, ENOENT, -1);
    
    for(; n; n--)
    {
        vec_pop_en(stack->stack, sizeof(type_id), &type);
        size = type_size(type);
        
        type_free(type, vec_at(stack->stack, -size));
        
        vec_pop_n(stack->stack, size);
        stack->size--;
    }
    
    return 0;
}

type_id parser_stack_type(parser_stack_ct stack, size_t pos)
{
    ssize_t offset;
    
    assert_nmagic(stack, MAGIC_STACK, NULL);
    
    if((offset = get_offset(stack, pos)) < 0)
        return TYPE_INVALID;
    
    return *(type_id*)vec_at(stack->stack, -offset - sizeof(type_id));
}

size_t parser_stack_size(parser_stack_ct stack, size_t pos)
{
    type_id type;
    
    if(!(type = parser_stack_type(stack, pos)))
        return 0;
    
    return type_size(type);
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

size_t parser_stack_results(parser_stack_ct stack)
{
    assert_nmagic(stack, MAGIC_STACK, 0);
    
    return stack->size;
}

void parser_stack_clear(parser_stack_ct stack)
{
    assert_nmagic(stack, MAGIC_STACK, );
    
    parser_stack_drop(stack, stack->size);
}
*/
