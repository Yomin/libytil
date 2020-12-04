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

#include <ytil/sys/env.h>
#include <ytil/con/art.h>
#include <ytil/def.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct env_value
{
    const char *def;
    str_const_ct set;
} env_value_st;

typedef struct env_fold_state
{
    env_fold_cb fold;
    void *ctx;
} env_fold_state_st;

/// env error type definition
ERROR_DEFINE_LIST(ENV,
      ERROR_INFO(E_ENV_CALLBACK, "Callback error.")
    , ERROR_INFO(E_ENV_INVALID_NAME, "Invalid environment name.")
    , ERROR_INFO(E_ENV_NOT_FOUND, "Environment value not found.")
);

/// default error type for env module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_ENV

static art_ct env;
extern char **environ;

#define ENV_UNSET ((str_const_ct)1)


int env_init(void)
{
    env_value_st *value;
    char **var, *sep;
    str_ct name = LIT("");
    
    if(env)
        return 0;
    
    if(!(env = art_new(ART_MODE_UNORDERED)))
        return error_wrap(), -1;
    
    for(var = environ; *var; var++)
    {
        if(!(sep = strchr(*var, '=')) || sep == *var)
            continue;
        
        if(!(value = calloc(1, sizeof(env_value_st))))
            return error_wrap_last_errno(calloc), env_free(), -1;
        
        sep[0] = '\0';
        str_set_sn(name, *var, sep - *var);
        
        if(!art_insert(env, name, value))
            return error_wrap(), free(value), env_free(), sep[0] = '=', -1;
        
        sep[0] = '=';
        value->def = &sep[1];
    }
    
    return 0;
}

static void env_free_value(env_value_st *value)
{
    if(value->set && value->set != ENV_UNSET)
        str_unref(value->set);
    
    free(value);
}

static void env_art_free_value(art_const_ct art, void *data, void *ctx)
{
    env_free_value(data);
}

void env_free(void)
{
    if(env)
    {
        art_free_f(env, env_art_free_value, NULL);
        env = NULL;
    }
}

bool env_is_set(str_const_ct name)
{
    env_value_st *value;
    
    return_error_if_pass(str_is_empty(name), E_ENV_INVALID_NAME, false);
    
    if(env_init())
        return error_pass(), false;
    
    return (value = art_get_data(env, name)) && value->set != ENV_UNSET;
}

int env_set(str_const_ct name, str_const_ct str)
{
    env_value_st *value;
    
    return_error_if_pass(str_is_empty(name), E_ENV_INVALID_NAME, -1);
    
    if(env_init())
        return error_pass(), -1;
    
    if(!(str = str_ref(str)))
        return error_wrap(), -1;
    
    if(!(value = art_get_data(env, name)))
    {
        if(!(value = calloc(1, sizeof(env_value_st))))
            return error_wrap_last_errno(calloc), str_unref(str), -1;
        
        if(!art_insert(env, name, value))
            return error_wrap(), free(value), str_unref(str), -1;
    }
    
    if(value->set && value->set != ENV_UNSET)
        str_unref(value->set);
    
    value->set = str;
    str_mark_const(str);
    
    return 0;
}

static str_const_ct env_value_get(env_value_st *value)
{
    if(!value->set && !(value->set = str_new_s(value->def)))
        return error_wrap(), NULL;
    
    return value->set;
}

str_const_ct env_get(str_const_ct name)
{
    env_value_st *value;
    
    return_error_if_pass(str_is_empty(name), E_ENV_INVALID_NAME, NULL);
    
    if(env_init())
        return error_pass(), NULL;
    
    if(!(value = art_get_data(env, name)))
        return error_push(E_ENV_NOT_FOUND), NULL;
    
    if(value->set == ENV_UNSET)
        return error_set(E_ENV_NOT_FOUND), NULL;
    
    return error_pass_ptr(env_value_get(value));
}

int env_reset(str_const_ct name)
{
    art_node_ct node;
    env_value_st *value;
    
    return_error_if_pass(str_is_empty(name), E_ENV_INVALID_NAME, -1);
    
    if(!env || !(node = art_get(env, name)))
        return 0;
    
    value = art_node_data(node);
    
    if(!value->set)
        return 0;
    
    if(!value->def)
    {
        env_free_value(value);
        art_remove(env, node);
    }
    else if(value->set == ENV_UNSET)
    {
        value->set = NULL;
    }
    else if(str_c(value->set) != value->def)
    {
        str_unref(value->set);
        value->set = NULL;
    }
    
    return 0;
}

int env_unset(str_const_ct name)
{
    art_node_ct node;
    env_value_st *value;
    
    return_error_if_pass(str_is_empty(name), E_ENV_INVALID_NAME, -1);
    
    if(env_init())
        return error_pass(), -1;
    
    if(!(node = art_get(env, name)))
        return 0;
    
    value = art_node_data(node);
    
    if(value->def)
    {
        if(value->set && value->set != ENV_UNSET)
            str_unref(value->set);
        
        value->set = ENV_UNSET;
    }
    else
    {
        env_free_value(value);
        art_remove(env, node);
    }
    
    return 0;
}

static int env_art_fold_value(art_const_ct art, str_const_ct key, void *data, void *ctx)
{
    env_fold_state_st *state = ctx;
    env_value_st *value = data;
    str_const_ct str;
    
    if(!(str = env_value_get(value)))
        return error_pass(), -1;
    
    if(str == ENV_UNSET)
        return 0;
    
    return error_pack_int(E_ENV_CALLBACK, state->fold(key, str, state->ctx));
}

int env_fold(env_fold_cb fold, const void *ctx)
{
    env_fold_state_st state = { .fold = fold, .ctx = (void *)ctx };
    
    assert(fold);
    
    if(env_init())
        return error_pass(), -1;
    
    return error_pick_int(E_ART_CALLBACK, art_fold_k(env, env_art_fold_value, &state));
}

static int env_dump_value(str_const_ct name, str_const_ct value, void *ctx)
{
    size_t *no = ctx;
    
    printf("%03zu %s = %s\n", *no, (char*)str_bc(name), str_c(value));
    
    no[0]++;
    
    return 0;
}

void env_dump(void)
{
    size_t no = 1;
    
    env_fold(env_dump_value, &no);
}
