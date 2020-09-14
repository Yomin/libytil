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

#ifndef YTIL_DB_STMT_H_INCLUDED
#define YTIL_DB_STMT_H_INCLUDED

#include <ytil/db/interface.h>


typedef int (*db_param_map_cb)(db_stmt_const_ct stmt, size_t index, const db_param_bind_st *param, void *ctx);

typedef int (*db_result_map_cb)(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *result, void *ctx);


/// Create new statement.
///
/// \param db       database
/// \param ctx      statement context
///
/// \returns                    db statement
/// \retval NULL/E_GENERIC_OOM  out of memory
db_stmt_ct db_stmt_new(db_ct db, const void *ctx);

/// Free statement.
///
/// \param stmt     statement
///
/// \retval 0                   success
/// \retval -1/E_DB_ILLEGAL     not possible while statement is executing
int db_stmt_finalize(db_stmt_ct stmt);

/// Get statement context.
///
/// \param stmt     statement
///
/// \returns        statement context
void *db_stmt_get_ctx(db_stmt_const_ct stmt);

/// Get statement database.
///
/// \param stmt     statement
///
/// \returns        database
db_ct db_stmt_get_db(db_stmt_const_ct stmt);

/// Get statement database interface.
///
/// \param stmt     statement
///
/// \returns        interface
const db_interface_st *db_stmt_get_interface(db_stmt_const_ct stmt);

/// Set/Unset if statement is executing.
///
/// \param stmt         statement
/// \param executing    true if statement is executing
///
/// \retval 0                   success
/// \retval -1/E_DB_ILLEGAL     statement is already executing
int db_stmt_set_executing(db_stmt_ct stmt, bool executing);

/// Check if statement is executing.
///
/// \param stmt         statement
///
/// \retval true        statement is executing
/// \retval false       statement is not executing
bool db_stmt_is_executing(db_stmt_const_ct stmt);

/// Prepare statement with SQL.
///
/// The backend implementation may use this if no prepared statements are available.
///
/// \param stmt     statement
/// \param sql      SQL
///
/// \retval 0                       success
/// \retval -1/E_DB_MALFORMED_SQL   malformed SQL
/// \retval -1/E_DB_MULTI_STMT      multi statements not supported
/// \retval -1/E_GENERIC_OOM        out of memory
int db_stmt_prepare(db_stmt_ct stmt, const char *sql);

/// Get SQL from statement.
///
/// \note The returned string is only valid until the next
///       invocation or statement finalization.
///
/// \param stmt     statement
///
/// \returns                    SQL
/// \retval NULL/E_GENERIC_OOM  out of memory
const char *db_stmt_sql(db_stmt_ct stmt, db_sql_id type);

/// Escape SQL string.
///
/// \note The returned string is only valid until the next
///       invocation or statement finalization.
///
/// \param stmt     statement
/// \param sql      SQL to escape
///
/// \returns                    escaped SQL
/// \retval NULL/E_GENERIC_OOM  out of memory
const char *db_stmt_escape_sql(db_stmt_ct stmt, const char *sql);

/// Initialize statement parameters.
///
/// The backend implementation may use this to track statement parameters.
///
/// \param stmt     statement
/// \param n        number of parameters
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int db_stmt_param_init(db_stmt_ct stmt, size_t n);

/// Bind parameter to statement.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param param    parameter bind infos
///
/// \retval 0                       success
/// \retval -1/E_DB_OUT_OF_BOUNDS   index is out of bounds
int db_stmt_param_bind(db_stmt_ct stmt, size_t index, const db_param_bind_st *param);

/// Get parameter from statement.
///
/// \param stmt     statement
/// \param index    parameter index
///
/// \returns                            parameter bind infos
/// \retval NULL/E_DB_OUT_OF_BOUNDS     index is out of bounds
const db_param_bind_st *db_stmt_param_get(db_stmt_const_ct stmt, size_t index);

/// Map over all parameters.
///
/// \param stmt     statement
/// \param map      map callback
/// \param ctx      \p map context
///
/// \retval 0                   success
/// \retval >0                  \p map rc
/// \retval <0/E_DB_CALLBACK    \p map error
int db_stmt_param_map(db_stmt_const_ct stmt, db_param_map_cb map, const void *ctx);

/// Initialize statement result fields.
///
/// The backend implementation may use this to track statement result fields.
///
/// \param stmt     statement
/// \param n        number of result fields
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int db_stmt_result_init(db_stmt_ct stmt, size_t n);

/// Bind result field to statement.
///
/// \param stmt     statement
/// \param index    result field index
/// \param result   result field bind infos
///
/// \retval 0                       success
/// \retval -1/E_DB_OUT_OF_BOUNDS   index is out of bounds
int db_stmt_result_bind(db_stmt_ct stmt, size_t index, const db_result_bind_st *result);

/// Get result field from statement.
///
/// \param stmt     statement
/// \param index    result field index
///
/// \returns                            result field bind infos
/// \retval NULL/E_DB_OUT_OF_BOUNDS     index is out of bounds
const db_result_bind_st *db_stmt_result_get(db_stmt_const_ct stmt, size_t index);

/// Map over all result fields.
///
/// \param stmt     statement
/// \param map      map callback
/// \param ctx      \p map context
///
/// \retval 0                   success
/// \retval >0                  \p map rc
/// \retval <0/E_DB_CALLBACK    \p map error
int db_stmt_result_map(db_stmt_const_ct stmt, db_result_map_cb map, const void *ctx);


#endif // ifndef YTIL_DB_STMT_H_INCLUDED
