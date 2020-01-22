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

#include <ytil/itf/obj.h>
#include <ytil/itf/itf.h>
#include <ytil/def.h>
#include <stdlib.h>

typedef struct obj_itf
{
    obj_size_cb size;
    obj_dup_cb dup;
    obj_free_cb free;
} obj_itf_st;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_OBJ_INVALID_TYPE, "Invalid type.")
};


static itf_id obj_itf(void)
{
    static itf_id id;
    
    if(!id)
        id = itf_new(LIT("obj"), free);
    
    return id;
}

static obj_itf_st *obj_instance(type_id type)
{
    obj_itf_st *itf;
    
    if(!(itf = itf_get(obj_itf(), type)))
        abort();
    
    return itf;
}

int obj_register(type_id type, obj_size_cb size, obj_dup_cb dup, obj_free_cb objfree)
{
    obj_itf_st *itf;
    
    if(!obj_itf())
        return error_wrap(), -1;
    
    if(!(itf = calloc(1, sizeof(obj_itf_st))))
        return error_wrap_errno(calloc), -1;
    
    itf->size = size;
    itf->dup = dup;
    itf->free = objfree;
    
    if(itf_register(obj_itf(), type, itf))
        return error_map(0, E_ITF_INVALID_TYPE, E_OBJ_INVALID_TYPE), free(itf), -1;
    
    return 0;
}

size_t obj_size(type_id type, obj_ct obj)
{
    return obj_instance(type)->size(obj);
}

obj_ct obj_dup(type_id type, obj_ct obj)
{
    return obj_instance(type)->dup(obj);
}

void obj_free(type_id type, obj_ct obj)
{
    obj_instance(type)->free(obj);
}
