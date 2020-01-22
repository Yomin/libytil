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

#include <ytil/itf/obj.h>
#include <ytil/def.h>
#include <ytil/con/vec.h>

//#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>

typedef struct obj
{
    str_const_ct name;
    
    obj_size_cb sizef;
    obj_free_cb free;
} type_st;

/*static const error_info_st error_infos[] =
{
      ERROR_INFO(E_TYPE_INVALID_CAPABILITY, "Invalid type capability.")
    , ERROR_INFO(E_TYPE_INVALID_NAME, "Invalid type name.")
    , ERROR_INFO(E_TYPE_INVALID_SIZE, "Invalid type size.")
    , ERROR_INFO(E_TYPE_INVALID_TYPE, "Invalid type.")
    , ERROR_INFO(E_TYPE_UNSUPPORTED, "Type does not support this capability.")
};*/

/*
static void vec_free_type(vec_const_ct vec, void *elem, void *ctx)
{
    type_st *type = elem;
    
    if(type->params)
        free(type->params);
    
    if(type->dtor)
        type->dtor(type->id, type->ctx);
}

void types_free(void)
{
    if(types.list)
        vec_free_f(types.list, vec_free_type, NULL);
    if(types.index)
        art_free(types.index);
    
    types.list = NULL;
    types.index = NULL;
}
*/
/*static type_id register_type(str_const_ct name, size_t size, type_size_cb sizef, void *ctx, type_dtor_cb dtor, va_list ap)
{
    type_st *type;
    art_node_ct node;
    type_cap_id cap;
    
    return_error_if_fail(name && !str_is_empty(name), E_TYPE_INVALID_NAME, TYPE_INVALID);
    return_error_if_fail(size || sizef, E_TYPE_INVALID_SIZE, TYPE_INVALID);
    
    if((!types.list && !(types.list = vec_new(2, sizeof(type_st))))
    || (!types.index && !(types.index = art_new(ART_MODE_UNORDERED))))
        return error_wrap(), TYPE_INVALID;
    
    if((type = art_get_data(types.index, name)))
    {
        return type->id; // todo
    }
    
    if(!(type = vec_push(types.list)))
        return error_wrap(), TYPE_INVALID;
    
    if(!(node = art_insert(types.index, name, type)))
        return error_wrap(), vec_pop(types.list), TYPE_INVALID;
    
    if(!(type->name = str_ref(name)))
        return error_wrap(), art_remove(types.index, node), vec_pop(types.list), TYPE_INVALID;
    
    type->id = TYPE_BASE_TYPES + vec_size(types.list) -1;
    type->size = size;
    type->sizef = sizef;
    type->ctx = ctx;
    type->dtor = dtor;
    
    while((cap = va_arg(ap, type_cap_id)) != TYPE_CAP_NONE)
        switch(cap)
        {
        case TYPE_CAP_CHECK:    type->check = va_arg(ap, type_check_cb); break;
        case TYPE_CAP_SHOW:     type->show = va_arg(ap, type_show_cb); break;
        case TYPE_CAP_CMP:      type->cmp = va_arg(ap, type_cmp_cb); break;
        case TYPE_CAP_EQUAL:    type->equal = va_arg(ap, type_equal_cb); break;
        case TYPE_CAP_FREE:     type->free = va_arg(ap, type_free_cb); break;
        default:                abort();
        }
    
    return type->id;
}*/
/*
size_t type_size(type_id id)
{
    type_st *type;
    
    switch(id)
    {
    case TYPE_INVALID:  return error_set(E_TYPE_INVALID_TYPE), 0;
    case TYPE_BOOL:     return sizeof(bool);
    case TYPE_CHAR:     return sizeof(char);
    case TYPE_UCHAR:    return sizeof(unsigned char);
    case TYPE_INT:      return sizeof(int);
    case TYPE_UINT:     return sizeof(unsigned int);
    case TYPE_LONG:     return sizeof(long);
    case TYPE_ULONG:    return sizeof(unsigned long);
    case TYPE_INT8:     return sizeof(int8_t);
    case TYPE_UINT8:    return sizeof(uint8_t);
    case TYPE_INT16:    return sizeof(int16_t);
    case TYPE_UINT16:   return sizeof(uint16_t);
    case TYPE_INT32:    return sizeof(int32_t);
    case TYPE_UINT32:   return sizeof(uint32_t);
    case TYPE_INT64:    return sizeof(int64_t);
    case TYPE_UINT64:   return sizeof(uint64_t);
    case TYPE_ENUM:     // fixme
    case TYPE_SIZE:     return sizeof(size_t);
    case TYPE_SSIZE:    return sizeof(ssize_t);
    case TYPE_FLOAT:    return sizeof(float);
    case TYPE_DOUBLE:   return sizeof(double);
    case TYPE_CSTRING:  return sizeof(char*);
    case TYPE_POINTER:  return sizeof(void*);
    default:
        if(!types.list || !(type = vec_at(types.list, id - TYPE_BASE_TYPES)))
            return error_set(E_TYPE_INVALID_TYPE), 0;
        if(!type->sizef)
            return type->size;
        return type->sizef(id, type->params, type->ctx);
    }
}

void type_free(type_id id, void *value)
{
    type_st *type;
    
    switch(id)
    {
    case TYPE_CSTRING:
        free(*(char**)value);
        break;
    default:
        if(id < TYPE_BASE_TYPES)
            break;
        if(!types.list || !(type = vec_at(types.list, id - TYPE_BASE_TYPES)))
            abort();
        if(type->free)
            type->free(id, type->params, value, type->ctx);
    }
}
*/
