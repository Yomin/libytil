/*
 * Copyright (c) 2020-2021 Martin Rödel aka Yomin
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

#include <ytil/itf/itf.h>
#include <ytil/def.h>
#include <ytil/con/vec.h>
#include <stdlib.h>


/// interface
typedef struct itf
{
    const char  *name;  ///< interface name
    vec_ct      types;  ///< registered type instances
    itf_dtor_cb dtor;   ///< interface type instance dtor
} itf_st;

/// interface list
static vec_ct itfs;

/// interface error type definition
ERROR_DEFINE_LIST(ITF,
    ERROR_INFO(E_ITF_INVALID_TYPE, "Invalid type."),
    ERROR_INFO(E_ITF_REGISTERED, "Type already registered."),
    ERROR_INFO(E_ITF_UNSUPPORTED, "Type does not support interface.")
);

/// default error type for interface module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_ITF


static void itf_vec_free_instance(vec_const_ct vec, void *instance, void *ctx)
{
    itf_ct itf = ctx;

    itf->dtor(instance);
}

static void itf_vec_free_itf(vec_const_ct vec, void *elem, void *ctx)
{
    itf_ct itf = elem;

    if(itf->types)
        vec_free_f(itf->types, itf->dtor ? itf_vec_free_instance : NULL, itf);
}

void itfs_free(void)
{
    if(itfs)
    {
        vec_free_f(itfs, itf_vec_free_itf, NULL);
        itfs = NULL;
    }
}

itf_ct itf_new(const char *name, itf_dtor_cb dtor)
{
    itf_ct itf;

    assert(name);

    if(!itfs && !(itfs = vec_new(2, sizeof(itf_st))))
        return error_wrap(), NULL;

    if(!(itf = vec_push(itfs)))
        return error_wrap(), NULL;

    itf->name   = name;
    itf->dtor   = dtor;

    return itf;
}

int itf_register(itf_ct itf, type_id type, void *instance)
{
    void **pinstance;

    assert(itfs && itf && vec_is_member(itfs, itf));
    return_error_if_fail(type_is_valid(type), E_ITF_INVALID_TYPE, -1);
    assert(instance);

    if(!itf->types && !(itf->types = vec_new(type + 1, sizeof(void*))))
        return error_wrap(), -1;

    if(vec_size(itf->types) <= type && !vec_push_n(itf->types, vec_size(itf->types) - type + 1))
        return error_wrap(), -1;

    pinstance = vec_at(itf->types, type);

    if(*pinstance)
        return error_set(E_ITF_REGISTERED), -1;

    *pinstance = instance;

    return 0;
}

const char *itf_name(itf_const_ct itf)
{
    assert(itfs && itf && vec_is_member(itfs, itf));

    return itf->name;
}

void *itf_get(itf_const_ct itf, type_id type)
{
    void *instance;

    assert(itfs && itf && vec_is_member(itfs, itf));
    return_error_if_fail(type_is_valid(type), E_ITF_INVALID_TYPE, NULL);

    if(!itf->types || !(instance = vec_at(itf->types, type-1)))
        return error_push(E_ITF_UNSUPPORTED), NULL;

    return instance;
}

void *itf_get_abort(itf_const_ct itf, type_id type)
{
    void *instance;

    if(!(instance = itf_get(itf, type)))
        abort();

    return instance;
}
