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

#ifndef YTIL_DB_TDS_H_INCLUDED
#define YTIL_DB_TDS_H_INCLUDED

#include <ytil/db/db.h>


/// TDS error type declaration
ERROR_DECLARE(TDS);


/// Connect to TDS database.
///
/// \param app          application name, may be NULL
/// \param host         hostname, may be NULL for localhost
/// \param user         user name, may be NULL for current user
/// \param password     user password, may be NULL
/// \param timeout      connection timeout in seconds, 0 = default, -1 = infinite
/// \param default_db   default database, may be NULL
///
/// \returns                            db handle
/// \retval NULL/E_DB_ACCESS_DENIED     access denied
/// \retval NULL/E_DB_CONNECTION        could not reach server
/// \retval NULL/E_DB_INCOMPATIBLE      TDS version not compatible with server
/// \retval NULL/E_DB_UNKNOWN_DATABASE  default DB unknown or access denied
/// \retval NULL/E_DB_UNKNOWN_HOST      host not found
/// \retval NULL/E_GENERIC_OOM          out of memory
db_ct db_tds_connect(const char *app, const char *host, const char *user, const char *password, int timeout, const char *default_db);


#endif // ifndef YTIL_DB_TDS_H_INCLUDED
