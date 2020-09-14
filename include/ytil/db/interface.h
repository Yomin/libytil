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

#ifndef YTIL_DB_INTERFACE_H_INCLUDED
#define YTIL_DB_INTERFACE_H_INCLUDED

#include <ytil/db/db.h>


/// DB parameter bind mode
typedef enum db_param_bind_mode
{
    DB_PARAM_BIND_FIX,              ///< bind static value
    DB_PARAM_BIND_TMP,              ///< bind transient value
    DB_PARAM_BIND_REF,              ///< bind value reference
} db_param_bind_id;

/// DB parameter bind object
typedef struct db_param_bind
{
    db_param_bind_id    mode;       ///< bind mode
    db_type_id          type;       ///< type of \p data
    const void          *data;      ///< data to insert
    size_t              vsize;      ///< size of \p data, valid for (fix|tmp)-mode
    const size_t        *rsize;     ///< size of \p data, valid for ref-mode, may be NULL for text
    const bool          *is_null;   ///< set on NULL data, may be NULL
} db_param_bind_st;

/// DB result bind mode
typedef enum db_result_bind_mode
{
    DB_RESULT_BIND_FIX,             ///< copy data into fix buffer
    DB_RESULT_BIND_TMP,             ///< expose temporary data
    DB_RESULT_BIND_DUP,             ///< allocate copy of data
} db_result_bind_id;

/// DB result bind object
typedef struct db_result_bind
{
    db_result_bind_id   mode;       ///< bind mode
    db_type_id          type;       ///< type of \p data
    void                *data;      ///< data to fill, may be NULL for fix mode
    size_t              cap;        ///< capacity of \p data
    size_t              *size;      ///< size to fill, may be NULL
    bool                *is_null;   ///< set on NULL field, may be NULL
} db_result_bind_st;

/// DB SQL type
typedef enum db_sql
{
    DB_SQL_PLAIN,       ///< SQL with placeholders
    DB_SQL_EXPANDED,    ///< SQL with expanded parameters
} db_sql_id;

/// DB name type
typedef enum db_name
{
    DB_NAME_DATABASE,   ///< database name
    DB_NAME_TABLE,      ///< table name
    DB_NAME_TABLE_ORG,  ///< original table name
    DB_NAME_FIELD,      ///< field name
    DB_NAME_FIELD_ORG,  ///< original field name
} db_name_id;


/// DB interface callback to close database backend
///
/// \param db       database to close
typedef void (*db_close_cb)(db_ct db);

/// DB interface callback to prepare statement
///
/// \param db       database
/// \param sql      SQL
///
/// \returns                        statement
/// \retval NULL/E_DB_MALFORMED_SQL malformed SQL
/// \retval NULL/E_DB_CONNECTION    connection error
/// \retval NULL/E_GENERIC_OOM      out of memory
typedef db_stmt_ct (*db_prepare_cb)(db_ct db, const char *sql);

/// DB interface callback to free statement
///
/// \param stmt     statement
typedef void (*db_finalize_cb)(db_stmt_ct stmt);

/// DB interface callback to get SQL from statement
///
/// The returned string must stay valid until
/// the next callback invocation or statement finalization.
///
/// \param stmt     statement
/// \param type     SQL type
///
/// \returns                        SQL
/// \retval NULL/E_DB_UNSUPPORTED   type not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
typedef const char *(*db_sql_cb)(db_stmt_ct stmt, db_sql_id type);

/// DB interface callback to execute statement
///
/// Execute the statement and fetch the first result row, if any,
/// into bound buffers, if any bound, even if \p record is NULL.
///
/// \param stmt     statement
/// \param record   callback to invoke on each result row, may be NULL
/// \param ctx      \p result context
///
/// \retval 0                   success
/// \retval -1/E_DB_CONSTRAINT  constraint violation
/// \retval -1/E_DB_FULL        database is full
/// \retval -1/E_DB_CALLBACK    \p record callback error
/// \retval -1/E_GENERIC_OOM    out of memory
typedef int (*db_exec_cb)(db_stmt_ct stmt, db_record_cb record, void *ctx);

/// DB interface callback to get number of statement parameters or result fields
///
/// \param stmt     statement
///
/// \returns                        number of statement parameters or result fields
/// \retval -1/E_DB_UNSUPPORTED     not supported by backend
typedef ssize_t (*db_count_cb)(db_stmt_const_ct stmt);

/// DB interface callback to bind statement parameter
///
/// \param stmt     statement
/// \param index    parameter index
/// \param bind     bind infos
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED_MODE    unsupported bind mode
/// \retval -1/E_DB_UNSUPPORTED_TYPE    unsupported type
/// \retval -1/E_GENERIC_OOM            out of memory
typedef int (*db_param_bind_cb)(db_stmt_ct stmt, size_t index, const db_param_bind_st *bind);

/// DB interface callback to bind result field
///
/// \param stmt     statement
/// \param index    field index
/// \param bind     bind infos
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED_MODE    unsupported bind mode
/// \retval -1/E_DB_UNSUPPORTED_TYPE    unsupported type
/// \retval -1/E_GENERIC_OOM            out of memory
typedef int (*db_result_bind_cb)(db_stmt_ct stmt, size_t index, const db_result_bind_st *bind);

/// DB interface callback to fetch result field
///
/// \param stmt     statement
/// \param index    field index
/// \param offset   value offset in bytes
///
/// \retval 0                       success
/// \retval -1/E_DB_OUT_OF_BOUNDS   index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE    offset is out of range
/// \retval -1/E_GENERIC_OOM        out of memory
typedef int (*db_result_fetch_cb)(db_stmt_const_ct stmt, size_t index, size_t offset);

/// DB interface callback to get result field type
///
/// \param stmt     statement
/// \param index    field index
///
/// \returns                                    result field type
/// \retval DB_TYPE_INVALID/E_DB_OUT_OF_BOUNDS  index is out of bounds
/// \retval DB_TYPE_INVALID/E_DB_UNKNOWN_TYPE   field type unknown
typedef db_type_id (*db_type_cb)(db_stmt_const_ct stmt, size_t index);

/// DB interface callback to get result field name
///
/// \param stmt     statement
/// \param index    field index
/// \param type     type of name
///
/// \returns                        name of database, table or field
/// \retval NULL/E_DB_NO_NAME       field is expression or sub query
/// \retval NULL/E_DB_OUT_OF_BOUNDS index is out of bounds
/// \retval NULL/E_DB_UNSUPPORTED   type not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
typedef const char *(*db_name_cb)(db_stmt_const_ct stmt, size_t index, db_name_id type);

/// DB interface callback to enable/disable tracing
///
/// \param db       database
/// \param enable   if true enable tracing, else disable tracing
///
/// \retval 0       success
typedef int (*db_trace_cb)(db_ct db, bool enable);

/// database interface
typedef struct db_interface
{
    db_close_cb         close;          ///< callback to close database backend
    db_prepare_cb       prepare;        ///< callback to prepare statement
    db_finalize_cb      finalize;       ///< callback to free statement
    db_sql_cb           sql;            ///< callback to get SQL from statement
    db_exec_cb          exec;           ///< callback to execute statement
    db_count_cb         param_count;    ///< callback to get number of statement parameters
    db_param_bind_cb    param_bind;     ///< callback to bind statement parameter
    db_count_cb         result_count;   ///< callback to get number of result fields
    db_result_bind_cb   result_bind;    ///< callback to bind result field
    db_result_fetch_cb  result_fetch;   ///< callback to fetch result field
    db_type_cb          result_type;    ///< callback to get type of result field
    db_name_cb          result_name;    ///< callback to get table/database/field name of result field
    db_trace_cb         trace;          ///< callback to enable/disable tracing
} db_interface_st;


/// Create new database object.
///
/// \param interface    interface callbacks
/// \param ctx          database context
///
/// \returns                    db
/// \retval NULL/E_GENERIC_OOM  out of memory
db_ct db_new(const db_interface_st *interface, const void *ctx);

/// Increase database statement reference counter.
///
/// \param db       database
void db_ref(db_ct db);

/// Decrease database statement reference counter.
///
/// \param db       database
void db_unref(db_ct db);

/// Get database context.
///
/// \param db       database
///
/// \returns        database context
void *db_get_ctx(db_const_ct db);

/// Get database interface.
///
/// \param db       database
///
/// \returns        interface
const db_interface_st *db_get_interface(db_const_ct db);

/// Invoke event callback.
///
/// \param db       database
/// \param sql      expanded statement SQL
void db_event(db_const_ct db, const char *sql);


#include <ytil/db/stmt.h>


#endif // ifndef YTIL_DB_INTERFACE_H_INCLUDED
