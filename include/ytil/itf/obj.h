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

#ifndef YTIL_ITF_OBJ_H_INCLUDED
#define YTIL_ITF_OBJ_H_INCLUDED

#include <ytil/itf/itf.h>
#include <ytil/gen/error.h>


/// object error
typedef enum obj_error
{
    E_OBJ_INVALID_TYPE  ///< invalid type
} obj_error_id;

/// object error type declaration
ERROR_DECLARE(OBJ);

/// object duplicate callback
///
/// \param dst  destination to place duplicated object into
/// \param src  pointer to source object
/// \param ctx  callback context
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
typedef int (*obj_dup_cb)(void *dst, const void *src, void *ctx);

/// object free callback
///
/// \param obj  pointer to object to free
/// \param ctx  callback context
typedef void (*obj_free_cb)(void *obj, void *ctx);

/// object callback context dtor callback
///
/// \param ctx  context to destroy
typedef void (*obj_dtor_cb)(void *ctx);


/// Register new object implementation for type.
///
/// \param type     type ID
/// \param dup      callback to duplicate object of type
/// \param free     callback to free object of type
///
/// \retval 0                       success
/// \retval -1/E_OBJ_INVALID_TYPE   invalid type
/// \retval -1/E_GENERIC_OOM        out of memory
int itf_obj_register(type_id type, obj_dup_cb dup, obj_free_cb free);

/// Register new object implementation for type with callback context.
///
/// \param type     type ID
/// \param dup      callback to duplicate object of type
/// \param free     callback to free object of type
/// \param ctx      callback context
/// \param dtor     \p ctx dtor
///
/// \retval 0                       success
/// \retval -1/E_OBJ_INVALID_TYPE   invalid type
/// \retval -1/E_GENERIC_OOM        out of memory
int itf_obj_register_x(type_id type, obj_dup_cb dup, obj_free_cb free, const void *ctx, obj_dtor_cb dtor);

/// Duplicate object.
///
/// \param type     object type
/// \param dst      destination to place duplicated object into
/// \param src      pointer to source object
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int obj_dup(type_id type, void *dst, const void *src);

/// Free object.
///
/// \param type     object type
/// \param obj      object
void obj_free(type_id type, void *obj);


#endif
