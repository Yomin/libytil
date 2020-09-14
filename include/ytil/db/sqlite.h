/*
 * Copyright (c) 2020 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_DB_SQLITE_H_INCLUDED
#define YTIL_DB_SQLITE_H_INCLUDED

#include <ytil/db/db.h>
#include <ytil/gen/path.h>


/// SQLite error type declaration
ERROR_DECLARE(SQLITE);


/// Open SQLite database.
///
/// \param file     database file
///
/// \returns                    db handle
/// \retval NULL/E_GENERIC_OOM  out of memory
db_ct db_sqlite_open(path_const_ct file);

/// Open temporary in-memory SQLite database.
///
/// \returns                    db handle
/// \retval NULL/E_GENERIC_OOM  out of memory
db_ct db_sqlite_open_memory(void);

/// Open private temporary on-disk SQLite database.
///
/// \returns                    db handle
/// \retval NULL/E_GENERIC_OOM  out of memory
db_ct db_sqlite_open_private(void);


#endif // ifndef YTIL_DB_SQLITE_H_INCLUDED
