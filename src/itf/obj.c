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

#include <ytil/itf/obj.h>
#include <ytil/def.h>
#include <stdlib.h>


/// object interface
typedef struct itf_obj
{
    obj_dup_cb  dup;    ///< duplicate callback
    obj_free_cb free;   ///< free callback
    void        *ctx;   ///< callback context
    obj_dtor_cb dtor;   ///< \p ctx dtor
} itf_obj_st;

/// type error type definition
ERROR_DEFINE_LIST(OBJ,
    ERROR_INFO(E_OBJ_INVALID_TYPE, "Invalid type.")
);

/// default error type for type module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_OBJ


static void itf_obj_free_instance(void *instance)
{
    itf_obj_st *obj = instance;

    if(obj->dtor)
        obj->dtor(obj->ctx);

    free(obj);
}

static itf_ct itf_obj(void)
{
    static itf_ct itf;

    if(!itf)
        itf = itf_new("obj", itf_obj_free_instance);

    return itf;
}

int itf_obj_register(type_id type, obj_dup_cb dup, obj_free_cb free)
{
    return error_pass_int(itf_obj_register_x(type, dup, free, NULL, NULL));
}

int itf_obj_register_x(type_id type, obj_dup_cb dup, obj_free_cb objfree, const void *ctx, obj_dtor_cb dtor)
{
    itf_ct itf;
    itf_obj_st *instance;

    assert(dup);
    assert(objfree);
    return_error_if_fail(type_is_valid(type), E_OBJ_INVALID_TYPE, -1);

    if(!(itf = itf_obj()))
        return error_wrap(), -1;

    if(!(instance = calloc(1, sizeof(itf_obj_st))))
        return error_wrap_last_errno(calloc), -1;

    instance->dup   = dup;
    instance->free  = objfree;
    instance->ctx   = (void *)ctx;
    instance->dtor  = dtor;

    if(itf_register(itf, type, instance))
        return error_wrap(), free(instance), -1;

    return 0;
}

int obj_dup(type_id type, void *dst, const void *src)
{
    itf_obj_st *itf = itf_get_abort(itf_obj(), type);

    return error_pass_int(itf->dup(dst, src, itf->ctx));
}

void obj_free(type_id type, void *obj)
{
    itf_obj_st *itf = itf_get_abort(itf_obj(), type);

    itf->free(obj, itf->ctx);
}
