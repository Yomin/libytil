/*
 * Copyright (c) 2016-2020 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_GEN_LOG_H_INCLUDED
#define YTIL_GEN_LOG_H_INCLUDED

#include <ytil/def/color.h>
#include <ytil/gen/str.h>
#include <ytil/gen/error.h>
#include <sys/types.h>
#include <stdio.h>


#define LOG_ALL_UNITS   0   ///< log unit ID for all units
#define LOG_ALL_TARGETS 0   ///< log target ID for all targets

/// log level
typedef enum log_level
{
    LOG_INVALID,    ///< invalid level, used as return value
    LOG_OFF,        ///< logging disabled
    LOG_CRIT,       ///< level for critical errors
    LOG_ERROR,      ///< level for non critical errors
    LOG_WARN,       ///< level for warnings
    LOG_NOTE,       ///< level for notices
    LOG_INFO,       ///< level for info messages
    LOG_DEBUG,      ///< level for debug messages
    LOG_TRACE,      ///< level for trace messages
    LOG_LEVELS,     ///< number of log levels
} log_level_id;

/// color mode
typedef enum log_color
{
    LOG_COLOR_OFF,      ///< disable colors
    LOG_COLOR_AUTO,     ///< decide on whether target is TTY
    LOG_COLOR_ON,       ///< always use colors
    LOG_COLOR_MODES,    ///< number of color modes
} log_color_id;

/// log error
typedef enum log_error
{
    E_LOG_CALLBACK,         ///< callback error
    E_LOG_EXISTS,           ///< log unit exists already
    E_LOG_FOPEN,            ///< fopen error
    E_LOG_INVALID_NAME,     ///< invalid unit or target name
    E_LOG_INVALID_STREAM,   ///< invalid stream
    E_LOG_NOT_FOUND,        ///< log unit unknown
} log_error_id;

/// log error type declaration
ERROR_DECLARE(LOG);


/// log unit/target fold callback
///
/// \param id       unit/target ID
/// \param name     unit/target name
/// \param ctx      callback context
///
/// \retval 0       continue fold
/// \retval <0      stop fold with error
/// \retval >0      stop fold
typedef int (*log_fold_cb)(size_t id, str_const_ct name, void *ctx);

/// log sink fold callback
///
/// \param unit         unit ID
/// \param unit_name    unit name
/// \param target       target ID
/// \param target_name  target name
/// \param level        log level
/// \param ctx          callback context
///
/// \retval 0           continue fold
/// \retval <0          stop fold with error
/// \retval >0          stop fold
typedef int (*log_sink_fold_cb)(size_t unit, str_const_ct unit_name, size_t target, str_const_ct target_name, log_level_id level, void *ctx);

/// log target hook callback
///
/// Can be configured per target and is called before
/// and after writing a message to this target.
///
/// \param id       target ID
/// \param name     target name
/// \param start    true before, false after writing a message
/// \param ctx      callback context
typedef void (*log_hook_cb)(size_t id, str_const_ct name, bool start, void *ctx);

/// log message prefix custom specifier callback
///
/// \param target   target stream
/// \param width    field width as used in printf
/// \param ctx      callback context
typedef void (*log_spec_cb)(FILE *target, int width, void *ctx);


/// Free log.
///
///
void log_free(void);

/// Add new log unit.
///
/// Unit IDs start at 1, 0 is safe to use as 'uninitialized'.
///
/// \param name     unit name
///
/// \returns                        new unit ID
/// \retval -1/E_LOG_INVALID_NAME   invalid unit name
/// \retval -1/E_LOG_EXISTS         log unit exists already
/// \retval -1/E_GENERIC_OOM        out of memory
ssize_t log_unit_add(str_const_ct name);

/// Get log unit by name.
///
/// \param name     unit name
/// \param exact    if false perform prefix search and return first match
///
/// \returns                        unit ID
/// \retval -1/E_LOG_NOT_FOUND      unit not found
ssize_t log_unit_get(str_const_ct name, bool exact);

/// Get log unit name.
///
/// \param unit     unit ID
///
/// \returns                        unit name
/// \retval NULL/E_LOG_NOT_FOUND    unit not found
str_const_ct log_unit_get_name(size_t unit);

/// Get maximum log level of unit sinks.
///
/// \param unit     unit ID
///
/// \returns                                log level
/// \retval LOG_INVALID/E_LOG_NOT_FOUND     unit not found
log_level_id log_unit_get_max_level(size_t unit);

/// Fold over all log units.
///
/// \param fold     callback to invoke on each unit
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval <0/E_LOG_CALLBACK   \p fold error
/// \retval >0                  \p fold rc
int log_unit_fold(log_fold_cb fold, const void *ctx);

/// Get number of log units.
///
/// \returns    number of log units
size_t log_units(void);

/// Add file log target.
///
/// \param name     target name, may be NULL to use file name instead
/// \param file     file name
/// \param append   if true open file in append mode
/// \param color    color mode
///
/// \returns                        new target ID
/// \retval -1/E_LOG_INVALID_NAME   invalid target name
/// \retval -1/E_LOG_FOPEN          failed to open file
/// \retval -1/E_GENERIC_OOM        out of memory
ssize_t log_target_add_file(str_const_ct name, str_const_ct file, bool append, log_color_id color);

/// Add stream log target.
///
/// \param name     target name
/// \param stream   stream
/// \param close    if true close stream on log_free
/// \param color    color mode
///
/// \returns                            new target ID
/// \retval -1/E_LOG_INVALID_NAME       invalid target name
/// \retval -1/E_LOG_INVALID_STREAM     invalid stream
/// \retval -1/E_GENERIC_OOM            out of memory
ssize_t log_target_add_stream(str_const_ct name, FILE *stream, bool close, log_color_id color);

/// Add stdout log target.
///
/// \param color    color mode
///
/// \returns                            new target id
/// \retval -1/E_LOG_INVALID_STREAM     invalid stream (stdout was closed)
/// \retval -1/E_GENERIC_OOM            out of memory
ssize_t log_target_add_stdout(log_color_id color);

/// Add stderr log target.
///
/// \param color    color mode
///
/// \returns                            new target id
/// \retval -1/E_LOG_INVALID_STREAM     invalid stream (stderr was closed)
/// \retval -1/E_GENERIC_OOM            out of memory
ssize_t log_target_add_stderr(log_color_id color);

/// Get log target by name.
///
/// \param name     target name
/// \param exact    if false perform prefix search and return first match
///
/// \returns                        target ID
/// \retval -1/E_LOG_NOT_FOUND      target not found
ssize_t log_target_get(str_const_ct name, bool exact);

/// Get log target name.
///
/// \param target     target ID
///
/// \returns                        target name
/// \retval NULL/E_LOG_NOT_FOUND    target not found
str_const_ct log_target_get_name(size_t target);

/// Remove log target.
///
/// \warning Do not cache target IDs.
///          Removing a target shifts all targets with higher ID down by one.
///
/// \param target   target ID
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  target not found
int log_target_remove(size_t target);

/// Set log target message hook.
///
/// Set target message hook which is called before and after writing a message to target.
///
/// \param target   target ID
/// \param hook     hook callback, may be NULL to unset
/// \param ctx      \p hook context
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  target not found
int log_target_set_hook(size_t target, log_hook_cb hook, const void *ctx);

/// Fold over all log targets.
///
/// \param fold     callback to invoke on each target
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval <0/E_LOG_CALLBACK   \p fold error
/// \retval >0                  \p fold rc
int log_target_fold(log_fold_cb fold, const void *ctx);

/// Get number of log targets.
///
/// \returns    number of log targets
size_t log_targets(void);

/// Set sink log level.
///
/// \param unit     unit ID, may be \a LOG_ALL_UNITS to set all units
/// \param target   target ID, may be \a LOG_ALL_TARGETS to set all targets
/// \param level    log level
///
/// \retval 0                       success
/// \retval -1/E_LOG_NOT_FOUND      unit or target not found
/// \retval -1/E_GENERIC_OOM        out of memory
int log_sink_set_level(size_t unit, size_t target, log_level_id level);

/// Get sink log level.
///
/// \param unit     unit ID
/// \param target   target ID
///
/// \returns                                log level
/// \retval LOG_INVALID/E_LOG_NOT_FOUND     unit or target not found
log_level_id log_sink_get_level(size_t unit, size_t target);

/// Fold over all sinks of log unit.
///
/// \param unit     unit ID
/// \param fold     callback to invoke on each unit
/// \param ctx      \p fold context
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
/// \retval <0/E_LOG_CALLBACK   \p fold error
/// \retval >0                  \p fold rc
int log_sink_fold(size_t unit, log_sink_fold_cb fold, const void *ctx);

/// Get number of sinks of log unit.
///
/// \param unit     unit ID
///
/// \returns                    number of sinks
/// \retval -1/E_LOG_NOT_FOUND  unit not found
ssize_t log_sinks(size_t unit);

/// Get log level by name.
///
/// \param name     log level name
/// \param exact    if false perform prefix search and return first match
///
/// \returns                                log level
/// \retval LOG_INVALID/E_LOG_NOT_FOUND     no log level name matched
log_level_id log_level_get(str_const_ct name, bool exact);

/// Get log level name.
///
/// \param level    log level
///
/// \returns        log level name
const char *log_level_get_name(log_level_id level);

/// Flush all log targets.
///
///
void log_flush(void);

/// Set log message prefix.
///
/// The prefix string is prepended before each log message.
/// If it contains special specifiers, they will be substituted
/// before writing the log message.
/// It is also possible to specify the field width like ^10u or ^-10u.
///
/// specifier | substitution
/// --------- | -----------
/// ^^        | single circumflex
/// ^c        | log level color sequence start
/// ^l        | log level name
/// ^p        | PID
/// ^r        | color sequence reset
/// ^t        | target name
/// ^u        | unit name
/// ^D        | date as YYYY-MM-DD
/// ^T        | time as HH:MM:SS
///
/// \param prefix   prefix string, may be NULL to unset
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int log_prefix_set(str_const_ct prefix);

/// Add custom log message prefix specifier.
///
/// \param spec     specifier character
/// \param write    callback to invoke to substitute specifier
/// \param ctx      \p write context
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
int log_prefix_add_spec(char spec, log_spec_cb write, const void *ctx);

/// Log message.
///
/// \param unit     unit ID
/// \param level    log level
/// \param msg      message format string
/// \param ...      \p msg arguments
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 3, 4)))
int log_msg(size_t unit, log_level_id level, const char *msg, ...);

/// Log message.
///
/// \param unit     unit ID
/// \param level    log level
/// \param msg      message format string
/// \param ap       \p msg arguments
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 3, 0)))
int log_msg_v(size_t unit, log_level_id level, const char *msg, va_list ap);

/// Log message with error.
///
/// \param unit     unit ID
/// \param level    log level
/// \param msg      message format string
/// \param ...      \p msg arguments
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 3, 4)))
int log_msg_e(size_t unit, log_level_id level, const char *msg, ...);

/// Log message with error.
///
/// \param unit     unit ID
/// \param level    log level
/// \param msg      message format string
/// \param ap       \p msg arguments
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 3, 0)))
int log_msg_ev(size_t unit, log_level_id level, const char *msg, va_list ap);

/// Log message on critical log level.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_crit(size_t unit, const char *msg, ...);

/// Log message on critical log level with error.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_crit_e(size_t unit, const char *msg, ...);

/// Log message on error log level.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_error(size_t unit, const char *msg, ...);

/// Log message on error log level with error.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_error_e(size_t unit, const char *msg, ...);

/// Log message on warning loglevel.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_warn(size_t unit, const char *msg, ...);

/// Log message on warning log level with error.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_warn_e(size_t unit, const char *msg, ...);

/// Log message on notice log level.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_note(size_t unit, const char *msg, ...);

/// Log message on notice log level with error.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_note_e(size_t unit, const char *msg, ...);

/// Log message on info log level.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_info(size_t unit, const char *msg, ...);

/// Log message on info log level with error.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_info_e(size_t unit, const char *msg, ...);

/// Log message on debug log level.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_debug(size_t unit, const char *msg, ...);

/// Log message on debug log level with error.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_debug_e(size_t unit, const char *msg, ...);

/// Log message on trace log level.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_trace(size_t unit, const char *msg, ...);

/// Log message on trace log level with error.
///
/// \param unit     unit ID
/// \param msg      message format string
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  unit not found
__attribute__((format(printf, 2, 3)))
int log_trace_e(size_t unit, const char *msg, ...);


#endif // ifndef YTIL_GEN_LOG_H_INCLUDED
