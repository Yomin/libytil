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

#ifndef YTIL_DB_MYSQL_H_INCLUDED
#define YTIL_DB_MYSQL_H_INCLUDED

#include <ytil/db/db.h>


/// MySQL error type declaration
ERROR_DECLARE(MYSQL);


/// Connect to MySQL database via TCP/IP.
///
/// \param host         hostname, may be NULL for localhost
/// \param port         port, if <= 0 default port is used
/// \param user         user name, may be NULL for current user
/// \param password     user password, may be NULL
/// \param default_db   default database, may be NULL
///
/// \returns                            db handle
/// \retval NULL/E_DB_ACCESS_DENIED     access denied
/// \retval NULL/E_DB_CONNECTION        could not reach server
/// \retval NULL/E_DB_UNKNOWN_DATABASE  default DB unknown or access denied
/// \retval NULL/E_DB_UNKNOWN_HOST      host not found
/// \retval NULL/E_DB_VERSION_MISMATCH  client - server version mismatch
/// \retval NULL/E_GENERIC_OOM          out of memory
db_ct db_mysql_connect(const char *host, int port, const char *user, const char *password, const char *default_db);

/// Connect to local MySQL database.
///
/// \param user         user name, may be NULL for current user
/// \param password     user password, may be NULL
/// \param default_db   default database, may be NULL
///
/// \returns                            db handle
/// \retval NULL/E_DB_ACCESS_DENIED     access denied
/// \retval NULL/E_DB_CONNECTION        no server with default parameters running
/// \retval NULL/E_DB_UNKNOWN_DATABASE  default DB unknown or access denied
/// \retval NULL/E_DB_VERSION_MISMATCH  client - server version mismatch
/// \retval NULL/E_GENERIC_OOM          out of memory
db_ct db_mysql_connect_local(const char *user, const char *password, const char *default_db);

/// Connect to local MySQL database via unix socket.
///
/// \param socket       unix socket path, may be NULL
/// \param user         user name, may be NULL for current user
/// \param password     user password, may be NULL
/// \param default_db   default database, may be NULL
///
/// \returns                            db handle
/// \retval NULL/E_DB_ACCESS_DENIED     access denied
/// \retval NULL/E_DB_CONNECTION        no server with specified socket running
/// \retval NULL/E_DB_UNKNOWN_DATABASE  default DB unknown or access denied
/// \retval NULL/E_DB_VERSION_MISMATCH  client - server version mismatch
/// \retval NULL/E_GENERIC_OOM          out of memory
db_ct db_mysql_connect_unix(const char *socket, const char *user, const char *password, const char *default_db);

/// Free MySQL resources after all MySQL connections are finished.
///
///
void db_mysql_free(void);


#endif // ifndef YTIL_DB_MYSQL_H_INCLUDED
