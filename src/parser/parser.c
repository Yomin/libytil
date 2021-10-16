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
#include <stdlib.h>


#define MAGIC define_magic("PAR") ///< parser magic

/// parser
typedef struct parser
{
    DEBUG_MAGIC

    size_t  ref;        ///< reference counter
    bool    floating;   ///< floating reference

    parser_parse_cb parse;  ///< parse callback
    void            *ctx;   ///< callback context
    parser_dtor_cb  dtor;   ///< callback context dtor
} parser_st;


/// parser error type definition
ERROR_DEFINE_LIST(PARSER,
    ERROR_INFO(E_PARSER_FAIL, "Parser failed."),
    ERROR_INFO(E_PARSER_ABORT, "Parser aborted."),
    ERROR_INFO(E_PARSER_ERROR, "Parser aborted with error."),
    ERROR_INFO(E_PARSER_STACK_MISSING, "Missing stack item."),
    ERROR_INFO(E_PARSER_STACK_TYPE, "Wrong stack item type requested."),
);

/// default error type for parser module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_PARSER


parser_ct parser_new(parser_parse_cb parse, const void *ctx, parser_dtor_cb dtor)
{
    parser_ct p;

    assert(parse);

    if(!(p = calloc(1, sizeof(parser_st))))
    {
        error_wrap_last_errno(calloc);

        if(dtor)
            dtor((void *)ctx);

        return NULL;
    }

    init_magic(p);

    p->ref      = 1;
    p->floating = true;

    p->parse    = parse;
    p->ctx      = (void *)ctx;
    p->dtor     = dtor;

    return p;
}

/// Free parser.
///
/// \param p    parser
static void parser_free(parser_ct p)
{
    if(p->dtor)
        p->dtor(p->ctx);

    free(p);
}

parser_ct parser_ref(parser_ct p)
{
    if(!p)
        return error_pass(), NULL;

    assert_magic(p);

    p->ref++;

    return p;
}

parser_ct parser_unref(parser_ct p)
{
    assert_magic(p);
    assert(p->ref && (!p->floating || p->ref > 1));

    if(--p->ref)
        return p;

    parser_free(p);

    return NULL;
}

parser_ct parser_sink(parser_ct p)
{
    if(!p)
        return error_pass(), NULL;

    assert_magic(p);
    assert(p->ref);

    if(!p->floating)
        return p;

    p->floating = false;

    if(--p->ref)
        return p;

    parser_free(p);

    return NULL;
}

parser_ct parser_ref_sink(parser_ct p)
{
    if(!p)
        return error_pass(), NULL;

    assert_magic(p);

    if(p->floating)
        p->floating = false;
    else
        p->ref++;

    return p;
}

size_t parser_get_ref_count(parser_const_ct p)
{
    assert_magic(p);

    return p->ref;
}

bool parser_is_floating(parser_const_ct p)
{
    assert_magic(p);

    return p->floating;
}

void *parser_get_ctx(parser_const_ct p)
{
    assert_magic(p);

    return p->ctx;
}

void parser_set_ctx(parser_ct p, const void *ctx)
{
    assert_magic(p);

    if(p->dtor)
        p->dtor(p->ctx);

    p->ctx = (void *)ctx;
}

ssize_t parser_parse(parser_const_ct p, const void *input, size_t len, parser_stack_ct stack, const void *state)
{
    ssize_t rc;

    assert_magic(p);
    assert(input);

    if(parser_stack_frame_push(stack))
        return error_pass(), -1;

    if((rc = p->parse(input, len, p->ctx, stack, (void *)state)) < 0)
        parser_stack_clear(stack);

    assert(rc < 0 || (size_t)rc <= len);

    parser_stack_frame_pop(stack);

    return error_pass_int(rc);
}



/*
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
