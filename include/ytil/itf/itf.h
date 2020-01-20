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

#ifndef YTIL_ITF_ITF_H_INCLUDED
#define YTIL_ITF_ITF_H_INCLUDED

#include <ytil/gen/type.h>
#include <ytil/gen/error.h>


/// interface error
typedef enum itf_error
{
    E_ITF_INVALID_TYPE,     ///< invalid type
    E_ITF_REGISTERED,       ///< type already registered
    E_ITF_UNSUPPORTED,      ///< type does not implement interface
} itf_error_id;

struct itf;
typedef       struct itf *itf_ct;       ///< interface type
typedef const struct itf *itf_const_ct; ///< const interface type

/// interface type instance dtor callback
///
/// \param instance     interface type instance
typedef void (*itf_dtor_cb)(void *instance);


/// Free all interfaces.
///
///
void itfs_free(void);

/// Create new interface.
///
/// \param name     interface name
/// \param dtor     callback to destroy interface type instances, may be NULL
///
/// \returns                    new interface
/// \retval NULL/E_GENERIC_OOM  out of memory
itf_ct itf_new(const char *name, itf_dtor_cb dtor);

/// Register new type for interface.
///
/// \param itf          interface
/// \param type         type ID
/// \param instance     interface type instance
///
/// \retval 0                       success
/// \retval -1/E_ITF_INVALID_TYPE   invalid type
/// \retval -1/E_ITF_REGISTERED     type already registered
/// \retval -1/E_GENERIC_OOM        out of memory
int itf_register(itf_ct itf, type_id type, void *instance);

/// Get interface name.
///
/// \param itf      interface
///
/// \returns        interface name
const char *itf_name(itf_const_ct itf);

/// Get interface type instance.
///
/// \param itf      interface
/// \param type     type ID
///
/// \returns                            interface type instance
/// \retval NULL/E_ITF_INVALID_TYPE     invalid type
/// \retval NULL/E_ITF_UNSUPPORTED      type does not implement interface
void *itf_get(itf_const_ct itf, type_id type);

/// Get interface type instance or abort.
///
/// \param itf      interface
/// \param type     type ID
///
/// \returns interface type instance
void *itf_get_abort(itf_const_ct itf, type_id type);


#endif
