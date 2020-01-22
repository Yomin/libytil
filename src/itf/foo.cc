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

#include <ytil/type/type.h>
#include <ytil/def.h>
#include <ytil/con/vec.h>
#include <ytil/con/art.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CMP(type, v1, v2) \
      (*(type*)v1) < (*(type*)v2) ? -1 \
    : (*(type*)v1) > (*(type*)v2) ? 1 : 0

typedef struct type_info
{
    type_id id;
    str_const_ct name;
    size_t size, nparams;
    type_id *params;
    
    type_check_cb check;
    type_size_cb sizef;
    type_show_cb show;
    type_cmp_cb cmp;
    type_equal_cb equal;
    type_free_cb free;
    
    void *ctx;
    type_dtor_cb dtor;
} type_st;

struct types
{
    vec_ct list;
    art_ct index;
};

static struct types types;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_TYPE_INVALID_CAPABILITY, "Invalid type capability.")
    , ERROR_INFO(E_TYPE_INVALID_NAME, "Invalid type name.")
    , ERROR_INFO(E_TYPE_INVALID_SIZE, "Invalid type size.")
    , ERROR_INFO(E_TYPE_INVALID_TYPE, "Invalid type.")
    , ERROR_INFO(E_TYPE_UNSUPPORTED, "Type does not support this capability.")
};

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
static type_id register_type(str_const_ct name, size_t size, type_size_cb sizef, void *ctx, type_dtor_cb dtor, va_list ap)
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
}

type_id _type_register(str_const_ct name, size_t size, ...)
{
    type_id type;
    va_list ap;
    
    va_start(ap, size);
    type = register_type(name, size, NULL, NULL, NULL, ap);
    va_end(ap);
    
    return type;
}

type_id _type_register_x(str_const_ct name, size_t size, void *ctx, type_dtor_cb dtor, ...)
{
    type_id type;
    va_list ap;
    
    va_start(ap, dtor);
    type = register_type(name, size, NULL, ctx, dtor, ap);
    va_end(ap);
    
    return type;
}
/*
type_id type_register_p(str_const_ct name, size_t size, size_t params, size_t caps, ...)
{
    type_id type;
    va_list ap;
    
    va_start(ap, caps);
    type = register_type(name, size, NULL, NULL, NULL, params, caps, ap);
    va_end(ap);
    
    return type;
}

type_id type_register_px(str_const_ct name, size_t size, void *ctx, type_dtor_cb dtor, size_t params, size_t caps, ...)
{
    type_id type;
    va_list ap;
    
    va_start(ap, caps);
    type = register_type(name, size, NULL, ctx, dtor, params, caps, ap);
    va_end(ap);
    
    return type;
}

type_id type_register_pf(str_const_ct name, type_size_cb sizef, size_t params, size_t caps, ...)
{
    type_id type;
    va_list ap;
    
    va_start(ap, caps);
    type = register_type(name, 0, sizef, NULL, NULL, params, caps, ap);
    va_end(ap);
    
    return type;
}

type_id type_register_pfx(str_const_ct name, type_size_cb sizef, void *ctx, type_dtor_cb dtor, size_t params, size_t caps, ...)
{
    type_id type;
    va_list ap;
    
    va_start(ap, caps);
    type = register_type(name, 0, sizef, ctx, dtor, ap);
    va_end(ap);
    
    return type;
}
*/
bool type_is_valid(type_id type)
{
    return type > TYPE_INVALID && (type < TYPE_BASE_TYPES
        || (types.list && type - TYPE_BASE_TYPES < vec_size(types.list)));
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
    case TYPE_ENUM:     return "enum";
    case TYPE_SIZE:     return "size";
    case TYPE_SSIZE:    return "ssize";
    case TYPE_FLOAT:    return "float";
    case TYPE_DOUBLE:   return "double";
    case TYPE_CSTRING:  return "cstring";
    case TYPE_POINTER:  return "pointer";
    default:
        if(!types.list || !(type = vec_at(types.list, id - TYPE_BASE_TYPES)))
            return error_set(E_TYPE_INVALID_TYPE), NULL;
        return str_c(type->name);
    }
}

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

str_ct type_show(type_id id, void *value)
{
/*    type_st *type;
    
    switch(id)
    {
    case TYPE_INVALID:  return error_set(E_TYPE_INVALID_TYPE), NULL;
    case TYPE_BOOL:     return str_new_l(*(bool*)value ? "true" : "false");
    case TYPE_CHAR:
    case TYPE_UCHAR:    return str_append_sc(str, *(char*)value, 1);
    case TYPE_INT:      return str_dup_f("%i", *(int*)value);
    case TYPE_UINT:     return str_dup_f("%u", *(unsigned int*)value);
    case TYPE_LONG:     return str_dup_f("%li", *(long*)value);
    case TYPE_ULONG:    return str_dup_f("%lu", *(unsigned long*)value);
    case TYPE_INT8:     return str_dup_f("%hhi", *(int8_t*)value);
    case TYPE_UINT8:    return str_dup_f("%hhu", *(uint8_t*)value);
    case TYPE_INT16:    return str_dup_f("%hi", *(int16_t*)value);
    case TYPE_UINT16:   return str_dup_f("%hu", *(uint16_t*)value);
    case TYPE_INT32:    return str_dup_f("%i", *(int32_t*)value);
    case TYPE_UINT32:   return str_dup_f("%u", *(uint32_t*)value);
    case TYPE_INT64:    return str_dup_f("%li", *(int64_t*)value);
    case TYPE_UINT64:   return str_dup_f("%lu", *(uint64_t*)value);
    case TYPE_ENUM:
    case TYPE_SIZE:     return str_dup_f("%zu", *(size_t*)value);
    case TYPE_SSIZE:    return str_dup_f("%zi", *(ssize_t*)value);
    case TYPE_FLOAT:    return str_dup_f("%g", *(float*)value);
    case TYPE_DOUBLE:   return str_dup_f("%lg", *(double*)value);
    case TYPE_CSTRING:  return str_append_c(str, *(char**)value);
    case TYPE_POINTER:  return str_dup_f("%p", *(void**)value);
    default:
        if(!types.list || !(type = vec_at(types.list, id - TYPE_BASE_TYPES)))
            return error_set(E_TYPE_INVALID_TYPE), NULL;
        if(!type->show
        || (type->check && !type->check(id, type->params, TYPE_CAP_SHOW, type->ctx)))
            return errno = ENOSYS, NULL;
        
        return type->show(str, id, type->params, value, type->ctx);
    }*/
    
    return NULL;
}

int type_cmp(type_id id, void *v1, void *v2)
{
    /*type_st *type;
    
    switch(id)
    {
    case TYPE_INVALID:  return -2;
    case TYPE_BOOL:     return CMP(bool, v1, v2);
    case TYPE_CHAR:     return CMP(char, v1, v2);
    case TYPE_UCHAR:    return CMP(unsigned char, v1, v2);
    case TYPE_INT:      return CMP(int, v1, v2);
    case TYPE_UINT:     return CMP(unsigned int, v1, v2);
    case TYPE_LONG:     return CMP(long, v1, v2);
    case TYPE_ULONG:    return CMP(unsigned long, v1, v2);
    case TYPE_INT8:     return CMP(int8_t, v1, v2);
    case TYPE_UINT8:    return CMP(uint8_t, v1, v2);
    case TYPE_INT16:    return CMP(int16_t, v1, v2);
    case TYPE_UINT16:   return CMP(uint16_t, v1, v2);
    case TYPE_INT32:    return CMP(int32_t, v1, v2);
    case TYPE_UINT32:   return CMP(uint32_t, v1, v2);
    case TYPE_INT64:    return CMP(int64_t, v1, v2);
    case TYPE_UINT64:   return CMP(uint64_t, v1, v2);
    case TYPE_ENUM:
    case TYPE_SIZE:     return CMP(size_t, v1, v2);
    case TYPE_SSIZE:    return CMP(ssize_t, v1, v2);
    case TYPE_FLOAT:    return CMP(float, v1, v2);
    case TYPE_DOUBLE:   return CMP(double, v1, v2);
    case TYPE_CSTRING:  return strcmp(*(char**)v1, *(char**)v2);
    default:
        if(id >= TYPE_BASE_TYPES)
        {
            if(!types.list || !(type = vec_at(types.list, id - TYPE_BASE_TYPES)))
                abort();
            if(type->check && !type->check(id, type->params, TYPE_CAP_CMP, type->ctx))
                return -2;
            if(type->cmp)
                return type->cmp(id, type->params, v1, v2, type->ctx);
        }
        
        return memcmp(v1, v2, type_size(id));
    }
    */
    
    return 0;
}

bool type_equal(type_id id, void *v1, void *v2)
{
    /*type_st *type;
    
    switch(id)
    {
    case TYPE_INVALID:  return false;
    case TYPE_FLOAT:    return *(float*)v1  == *(float*)v2;
    case TYPE_DOUBLE:   return *(double*)v1 == *(double*)v2;
    case TYPE_CSTRING:  return !strcmp(*(char**)v1, *(char**)v2);
    default:
        if(id >= TYPE_BASE_TYPES)
        {
            if(!types.list || !(type = vec_at(types.list, id - TYPE_BASE_TYPES)))
                abort();
            if(type->check
            && (!type->check(id, type->params, TYPE_CAP_EQUAL, type->ctx)
            ||  !type->check(id, type->params, TYPE_CAP_CMP, type->ctx)))
                return false;
            if(type->equal)
                return type->equal(id, type->params, v1, v2, type->ctx);
            if(type->cmp)
                return !type->cmp(id, type->params, v1, v2, type->ctx);
        }
        
        return !memcmp(v1, v2, type_size(id));
    }
    */
    
    return false;
}

void type_free(type_id id, void *value)
{
    /*type_st *type;
    
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
    }*/
}

bool type_check(type_id id, type_cap_id cap)
{
    /*type_st *type;
    
    assert_fail(cap < TYPE_CAPS, EINVAL, false);
    
    if(id < TYPE_BASE_TYPES)
        switch(cap)
        {
        case TYPE_CAP_CHECK:
        case TYPE_CAP_SHOW:
        case TYPE_CAP_CMP:
        case TYPE_CAP_EQUAL:
            return true;
        case TYPE_CAP_FREE:
            return id == TYPE_CSTRING;
        default:
            abort();
        }
    
    if(!types.list || !(type = vec_at(types.list, id - TYPE_BASE_TYPES)))
        abort();
    
    if(type->check && !type->check(id, type->params, cap, type->ctx))
        return false;
    
    switch(cap)
    {
    case TYPE_CAP_CHECK:    return true;
    case TYPE_CAP_SHOW:     return !!type->show;
    case TYPE_CAP_CMP:      return !!type->cmp;
    case TYPE_CAP_EQUAL:    return type->equal || type->cmp;
    case TYPE_CAP_FREE:     return !!type->free;
    default:                abort();
    }*/
    
    return false;
}
