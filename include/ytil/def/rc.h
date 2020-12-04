/*
 * Copyright (c) 2017-2020 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_DEF_RC_H_INCLUDED
#define YTIL_DEF_RC_H_INCLUDED

#include <ytil/gen/error.h>


/// Return if condition evaluates to true.
///
/// \param condition    condition to check
#define return_if_pass(condition) do { \
    if((condition))                    \
        return;                        \
} while(0)

/// Return if condition evaluates to false.
///
/// \param condition    condition to check
#define return_if_fail(condition) do { \
    if(!(condition))                   \
        return;                        \
} while(0)

/// Return value if condition evaluates to true.
///
/// \param condition    condition to check
/// \param value        value to return
#define return_value_if_pass(condition, value) do { \
    if((condition))                                 \
        return (value);                             \
} while(0)

/// Return value if condition evaluates to false.
///
/// \param condition    condition to check
/// \param value        value to return
#define return_value_if_fail(condition, value) do { \
    if(!(condition))                                \
        return (value);                             \
} while(0)

/// Set error and return value if condition evaluates to true.
///
/// \param condition    condition to check
/// \param error        error to set
/// \param value        value to return
#define return_error_if_pass(condition, error, value) do { \
    if((condition))                                        \
    {                                                      \
        error_set((error));                                \
        return (value);                                    \
    }                                                      \
} while(0)

/// Set error and return value if condition evaluates to false.
///
/// \param condition    condition to check
/// \param error        error to set
/// \param value        value to return
#define return_error_if_fail(condition, error, value) do { \
    if(!(condition))                                       \
    {                                                      \
        error_set((error));                                \
        return (value);                                    \
    }                                                      \
} while(0)

/// Set error and return value if statement is reached.
///
/// \param error        error to set
/// \param value        value to return
#define return_error_if_reached(error, value) do { \
    error_set((error));                            \
    return (value);                                \
} while(0)

/// Goto label if condition evaluates to true.
///
/// \param condition    condition to check
/// \param label        label to goto to
#define goto_if_pass(condition, label) do { \
    if((condition))                         \
        goto label;                         \
} while(0)

/// Goto label if condition evaluates to false.
///
/// \param condition    condition to check
/// \param label        label to goto to
#define goto_if_fail(condition, label) do { \
    if(!(condition))                        \
        goto label;                         \
} while(0)

/// Set error and goto label if condition evaluates to true.
///
/// \param condition    condition to check
/// \param error        error to set
/// \param label        label to goto to
#define goto_error_if_pass(condition, error, label) do { \
    if((condition))                                      \
    {                                                    \
        error_set((error));                              \
        goto label;                                      \
    }                                                    \
} while(0)

/// Set error and goto label if condition evaluates to false.
///
/// \param condition    condition to check
/// \param error        error to set
/// \param label        label to goto to
#define goto_error_if_fail(condition, error, label) do { \
    if(!(condition))                                     \
    {                                                    \
        error_set((error));                              \
        goto label;                                      \
    }                                                    \
} while(0)

/// Set error and goto label if statement is reached.
///
/// \param error        error to set
/// \param label        label to goto to
#define goto_error_if_reached(error, label) do { \
    error_set((error));                          \
    goto label;                                  \
} while(0)


#endif // ifndef YTIL_DEF_RC_H_INCLUDED
