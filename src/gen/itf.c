/*
 * Copyright (c) 2020 Martin Rödel aka Yomin
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

#include <ytil/gen/itf.h>
#include <ytil/def.h>
#include <ytil/con/vec.h>

typedef struct itf
{
    str_const_ct name;
    vec_ct types;
    itf_dtor_cb dtor;
} itf_st;

static vec_ct itfs;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_ITF_INVALID_INTERFACE, "Invalid interface.")
    , ERROR_INFO(E_ITF_INVALID_NAME, "Invalid interface name.")
    , ERROR_INFO(E_ITF_INVALID_TYPE, "Invalid type.")
    , ERROR_INFO(E_ITF_UNSUPPORTED, "Type does not support interface.")
};


static void itf_vec_free_instance(vec_const_ct vec, void *instance, void *ctx)
{
    itf_st *itf = ctx;
    
    itf->dtor(instance);
}

static void itf_vec_free_itf(vec_const_ct vec, void *elem, void *ctx)
{
    itf_st *itf = elem;
    
    str_unref(itf->name);
    
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

itf_id itf_new(str_const_ct name, itf_dtor_cb dtor)
{
    itf_st *itf;
    
    return_error_if_fail(name && !str_is_empty(name), E_ITF_INVALID_NAME, ITF_INVALID);
    
    if(!itfs && !(itfs = vec_new(2, sizeof(itf_st))))
        return error_wrap(), ITF_INVALID;
    
    if(!(itf = vec_push(itfs)))
        return error_wrap(), ITF_INVALID;
    
    if(!(itf->name = str_ref(name)))
        return error_wrap(), vec_pop(itfs), ITF_INVALID;
    
    itf->dtor = dtor;
    
    return vec_size(itfs);
}

static inline itf_st *_itf_get(itf_id id)
{
    return itfs && id ? vec_at(itfs, id-1) : NULL;
}

int itf_register(itf_id id, type_id type, void *instance)
{
    itf_st *itf;
    void **pinstance;
    
    return_error_if_fail(itf = _itf_get(id), E_ITF_INVALID_INTERFACE, -1);
    return_error_if_fail(type_is_valid(type), E_ITF_INVALID_TYPE, -1);
    assert(instance);
    
    if(!itf->types && !(itf->types = vec_new(type+1, sizeof(void*))))
        return error_wrap(), -1;
    
    if(vec_size(itf->types) <= type && !vec_push_n(itf->types, vec_size(itf->types)-type+1))
        return error_wrap(), -1;
    
    pinstance = vec_at(itf->types, type);
    
    if(*pinstance)
        return error_set(E_ITF_REGISTERED), -1;
    
    *pinstance = instance;
    
    return 0;
}

str_const_ct itf_name(itf_id id)
{
    itf_st *itf;
    
    if(!(itf = _itf_get(id)))
        return error_push(E_ITF_INVALID_INTERFACE), NULL;
    
    return itf->name;
}

void *itf_get(itf_id id, type_id type)
{
    itf_st *itf;
    void *imp;
    
    if(!(itf = _itf_get(id)))
        return error_push(E_ITF_INVALID_INTERFACE), NULL;
    
    if(!type)
        return error_set(E_ITF_INVALID_TYPE), NULL;
    
    if(!itf->types || !(imp = vec_at(itf->types, type-1)))
        return error_push(E_ITF_UNSUPPORTED), NULL;
    
    return imp;
}
