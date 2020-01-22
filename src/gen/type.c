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

#include <ytil/gen/type.h>
#include <ytil/def.h>
#include <ytil/con/vec.h>

//#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>

typedef struct type
{
    type_id id;
    str_const_ct name;
} type_st;

static vec_ct types;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_TYPE_INVALID_NAME, "Invalid type name.")
    , ERROR_INFO(E_TYPE_INVALID_TYPE, "Invalid type.")
};


static void vec_free_type(vec_const_ct vec, void *elem, void *ctx)
{
    type_st *type = elem;
    
    str_unref(type->name);
}

void types_free(void)
{
    if(types)
    {
        vec_free_f(types, vec_free_type, NULL);
        types = NULL;
    }
}

type_id type_register(str_const_ct name)
{
    type_st *type;
    
    return_error_if_fail(name && !str_is_empty(name), E_TYPE_INVALID_NAME, TYPE_INVALID);
    
    if(!types && !(types = vec_new(2, sizeof(type_st))))
        return error_wrap(), TYPE_INVALID;
    
    if(!(type = vec_push(types)))
        return error_wrap(), TYPE_INVALID;
    
    if(!(type->name = str_ref(name)))
        return error_wrap(), vec_pop(types), TYPE_INVALID;
    
    type->id = TYPE_BASE_TYPES + vec_size(types) -1;
    
    return type->id;
}

bool type_is_valid(type_id type)
{
    return type > TYPE_INVALID && (type < TYPE_BASE_TYPES
        || (types && type - TYPE_BASE_TYPES < vec_size(types)));
}

const char *type_name(type_id id)
{
    type_st *type;
    
    switch(id)
    {
    case TYPE_INVALID:  return error_set(E_TYPE_INVALID_TYPE), NULL;
    case TYPE_BOOL:     return "bool";
    case TYPE_CHAR:     return "char";
    case TYPE_UCHAR:    return "uchar";
    case TYPE_INT:      return "int";
    case TYPE_UINT:     return "uint";
    case TYPE_LONG:     return "long";
    case TYPE_ULONG:    return "ulong";
    case TYPE_INT8:     return "int8";
    case TYPE_UINT8:    return "uint8";
    case TYPE_INT16:    return "int16";
    case TYPE_UINT16:   return "uint16";
    case TYPE_INT32:    return "int32";
    case TYPE_UINT32:   return "uint32";
    case TYPE_INT64:    return "int64";
    case TYPE_UINT64:   return "uint64";
    case TYPE_SIZE:     return "size";
    case TYPE_SSIZE:    return "ssize";
    case TYPE_FLOAT:    return "float";
    case TYPE_DOUBLE:   return "double";
    case TYPE_CSTRING:  return "cstring";
    case TYPE_POINTER:  return "pointer";
    default:
        if(!types || !(type = vec_at(types, id - TYPE_BASE_TYPES)))
            return error_set(E_TYPE_INVALID_TYPE), NULL;
        return str_c(type->name);
    }
}
