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

#ifndef YTIL_DB_DB_H_INCLUDED
#define YTIL_DB_DB_H_INCLUDED

#include <ytil/gen/error.h>
#include <stddef.h>
#include <sys/types.h>
#include <inttypes.h>


/// DB error
typedef enum db_error
{
    E_DB_ACCESS_DENIED,         ///< access denied
    E_DB_BUSY,                  ///< server is busy
    E_DB_CALLBACK,              ///< callback error
    E_DB_CONNECTION,            ///< database connection error
    E_DB_CONSTRAINT,            ///< constraint violation
    E_DB_EXTENDED,              ///< extended backend error, check next lower error
    E_DB_FULL,                  ///< database is full
    E_DB_ILLEGAL,               ///< illegal operation
    E_DB_MALFORMED_SQL,         ///< malformed SQL
    E_DB_MAX_CONNECTIONS,       ///< max connections
    E_DB_MULTI_STMT,            ///< multi statements unsupported by backend
    E_DB_NO_DB,                 ///< no database selected
    E_DB_NO_NAME,               ///< no name for result field available
    E_DB_OUT_OF_BOUNDS,         ///< index out of bounds
    E_DB_OUT_OF_RANGE,          ///< parameter out of range
    E_DB_TYPE_MISMATCH,         ///< type conversion not possible
    E_DB_UNKNOWN_DATABASE,      ///< database unknown or access denied
    E_DB_UNKNOWN_HOST,          ///< database host unknown
    E_DB_UNKNOWN_LANGUAGE,      ///< language unknown
    E_DB_UNKNOWN_TYPE,          ///< result type unknown
    E_DB_UNSUPPORTED,           ///< unsupported function
    E_DB_UNSUPPORTED_MODE,      ///< unsupported bind mode
    E_DB_UNSUPPORTED_TYPE,      ///< unsupported type
    E_DB_VERSION_MISMATCH,      ///< client - server version mismatch
} db_error_id;

/// DB error type declaration
ERROR_DECLARE(DB);

/// DB type
typedef enum db_type
{
    DB_TYPE_INVALID,    ///< invalid type
    DB_TYPE_NULL,       ///< NULL
    DB_TYPE_BOOL,       ///< boolean
    DB_TYPE_INT8,       ///< 8 bit signed integer
    DB_TYPE_INT16,      ///< 16 bit signed integer
    DB_TYPE_INT32,      ///< 32 bit signed integer
    DB_TYPE_INT64,      ///< 64 bit signed integer
    DB_TYPE_UINT8,      ///< 8 bit unsigned integer
    DB_TYPE_UINT16,     ///< 16 bit unsigned integer
    DB_TYPE_UINT32,     ///< 32 bit unsigned integer
    DB_TYPE_UINT64,     ///< 64 bit unsigned integer
    DB_TYPE_FLOAT,      ///< float
    DB_TYPE_DOUBLE,     ///< double
    DB_TYPE_LDOUBLE,    ///< long double
    DB_TYPE_ID,         ///< ID
    DB_TYPE_TEXT,       ///< text
    DB_TYPE_BLOB,       ///< BLOB
    DB_TYPE_DATE,       ///< date
    DB_TYPE_TIME,       ///< time
    DB_TYPE_DATETIME,   ///< date and time
    DB_TYPE_TIMESTAMP,  ///< unix timestamp
} db_type_id;

/// DB date
typedef struct db_date
{
    uint16_t year;  ///< year
    uint8_t month;  ///< month
    uint8_t day;    ///< day
} db_date_st;

/// DB time
typedef struct db_time
{
    uint8_t hour;   ///< hour
    uint8_t minute; ///< minute
    uint8_t second; ///< second
} db_time_st;

/// DB datetime
typedef struct db_datetime
{
    db_date_st  date;   ///< date part
    db_time_st  time;   ///< time part
} db_datetime_st;

struct db;
typedef       struct db *db_ct;         ///< DB type
typedef const struct db *db_const_ct;   ///< const DB type

struct db_stmt;
typedef       struct db_stmt    *db_stmt_ct;        ///< DB statement type
typedef const struct db_stmt    *db_stmt_const_ct;  ///< const DB statement type


/// DB event callback
///
/// \param sql      expanded SQL of statement being executed
/// \param ctx      callback context
typedef void (*db_event_cb)(const char *sql, void *ctx);

/// DB record callback
///
/// \param stmt     statement being executed
/// \param row      row index
/// \param ctx      callback context
///
/// \retval 0       continue with next result row
/// \retval <0      stop execution with error
/// \retval >0      stop execution
typedef int (*db_record_cb)(db_stmt_ct stmt, size_t row, void *ctx);


/// Close database.
///
/// \param db   database
///
/// \retval 0                       success
/// \retval -1/E_DB_ILLEGAL         DB is still referenced by statements
int db_close(db_ct db);

/// Prepare statement.
///
/// \param db       database
/// \param sql      SQL
///
/// \returns                        statement
/// \retval NULL/E_DB_CONNECTION    connection error
/// \retval NULL/E_DB_MALFORMED_SQL malformed SQL
/// \retval NULL/E_DB_MULTI_STMT    multi statements not supported
/// \retval NULL/E_DB_NO_DB         no database selected
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
db_stmt_ct db_prepare(db_ct db, const char *sql);

/// Prepare statement from format string.
///
/// \param db       database
/// \param sql      SQL format string
/// \param ...      variadic list of \p sql arguments
///
/// \returns                        statement
/// \retval NULL/E_DB_CONNECTION    connection error
/// \retval NULL/E_DB_MALFORMED_SQL malformed SQL
/// \retval NULL/E_DB_MULTI_STMT    multi statements not supported
/// \retval NULL/E_DB_NO_DB         no database selected
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
db_stmt_ct db_prepare_f(db_ct db, const char *sql, ...);

/// Free prepared statement.
///
/// \param stmt     statement
///
/// \returns 0                      success
/// \retval -1/E_DB_ILLEGAL         not possible while statement is executing
int db_finalize(db_stmt_ct stmt);

/// Get SQL from statement.
///
/// \note The returned string is only valid until the next
///       db_sql* invocation or statement finalization.
///
/// \param stmt     statement
///
/// \returns                        SQL
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
const char *db_sql(db_stmt_ct stmt);

/// Get escaped SQL from statement.
///
/// \note The returned string is only valid until the next
///       db_sql* invocation or statement finalization.
///
/// \param stmt     statement
///
/// \returns                        SQL
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
const char *db_sql_e(db_stmt_ct stmt);

/// Get expanded SQL from statement.
///
/// \note The returned string is only valid until the next
///       db_sql* invocation or statement finalization.
///
/// \param stmt     statement
///
/// \returns                        SQL
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
const char *db_sql_expanded(db_stmt_ct stmt);

/// Get expanded and escaped SQL from statement.
///
/// \note The returned string is only valid until the next
///       db_sql* invocation or statement finalization.
///
/// \param stmt     statement
///
/// \returns                        SQL
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
const char *db_sql_expanded_e(db_stmt_ct stmt);

/// Execute prepared statement.
///
/// If the statement execution returned any result rows,
/// the first is fetched into bound buffers and the rest are dropped.
///
/// \param stmt     statement
///
/// \retval 0                       success
/// \retval -1/E_DB_CONSTRAINT      constraint violation
/// \retval -1/E_DB_FULL            database is full
/// \retval -1/E_DB_ILLEGAL         statement is already being executed
/// \retval -1/E_DB_UNSUPPORTED     not supported by backend
/// \retval -1/E_GENERIC_OOM        out of memory
int db_exec(db_stmt_ct stmt);

/// Execute prepared statement and invoke callback on each result row.
///
/// If the statement execution returned any result rows and record is NULL,
/// the first is fetched into bound buffers and the rest are dropped.
///
/// \param stmt     statement
/// \param record   callback to invoke on each result row, may be NULL
/// \param ctx      \p result context
///
/// \retval 0                       success
/// \retval -1/E_DB_CALLBACK        \p record callback error
/// \retval -1/E_DB_CONSTRAINT      constraint violation
/// \retval -1/E_DB_FULL            database is full
/// \retval -1/E_DB_ILLEGAL         statement is already being executed
/// \retval -1/E_DB_UNSUPPORTED     not supported by backend
/// \retval -1/E_GENERIC_OOM        out of memory
int db_exec_f(db_stmt_ct stmt, db_record_cb record, const void *ctx);

/// Enable statement tracing.
///
/// \param db       database
/// \param event    callback to invoke before executing a statement,
///                 may be NULL to disable tracing
/// \param ctx      \p trace context
///
/// \retval 0                       success
/// \retval -1/E_DB_UNSUPPORTED     tracing not supported by backend
int db_trace(db_ct db, db_event_cb event, const void *ctx);


#include <ytil/db/param.h>
#include <ytil/db/result.h>


#endif // ifndef YTIL_DB_DB_H_INCLUDED
