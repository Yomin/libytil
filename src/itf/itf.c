/*
 * Copyright (c) 2018-2020 Martin Rödel aka Yomin
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

#include <ytil/itf/itf.h>
#include <ytil/def.h>
#include <ytil/con/vec.h>

typedef struct itf
{
    str_const_ct name;
} itf_st;

static vec_ct itfs;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_ITF_INVALID_NAME, "Invalid interface name.")
};


static void itf_vec_free_itf(vec_const_ct vec, void *elem, void *ctx)
{
    itf_st *itf = elem;
    
    str_unref(itf->name);
}

void itf_free(void)
{
    if(itfs)
    {
        vec_free_f(itfs, itf_vec_free_itf, NULL);
        itfs = NULL;
    }
}

itf_ct itf_new(str_const_ct name)
{
    itf_st *itf;
    
    return_error_if_fail(name && !str_is_empty(name), E_ITF_INVALID_NAME, NULL);
    
    if(!itfs && !(itfs = vec_new(2, sizeof(itf_st))))
        return error_wrap(), NULL;
    
    if(!(itf = vec_push(itfs)))
        return error_wrap(), NULL;
    
    if(!(itf->name = str_ref(name)))
        return error_wrap(), vec_pop(itfs), NULL;
    
    return itf;
}

str_const_ct itf_name(itf_ct itf)
{
    return itf->name;
}
