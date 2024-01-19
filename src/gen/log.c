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

#include <ytil/gen/log.h>
#include <ytil/con/vec.h>
#include <ytil/ext/time.h>
#include <ytil/def.h>
#include <unistd.h>
#include <stdlib.h>


/// log unit
typedef struct log_unit
{
    str_const_ct    name;   ///< unit name
    vec_ct          sinks;  ///< unit sinks
    log_level_id    level;  ///< max log level of sinks
} log_unit_st;

/// log target
typedef struct log_target
{
    str_const_ct name;      ///< target name

    FILE    *stream;        ///< stream
    bool    close;          ///< if true close stream on log_free
    bool    color;          ///< if true write colored messages to target

    log_hook_cb hook;       ///< callback to run before and after writing to target
    void        *ctx;       ///< callback context
} log_target_st;

/// log sink
typedef struct log_sink
{
    log_target_st   *target;    ///< log target
    log_level_id    level;      ///< log level
} log_sink_st;

/// log message prefix specifier
typedef struct log_spec
{
    char        spec;   ///< specifier character
    log_spec_cb write;  ///< callback to invoke to substitute specifier
    void        *ctx;   ///< callback context
} log_spec_st;

/// log
typedef struct log
{
    vec_ct          units;      ///< unit list
    vec_ct          targets;    ///< target list
    str_const_ct    prefix;     ///< log message prefix
    vec_ct          specs;      ///< custom log message prefix specifiers
} log_st;

/// log state
static log_st log;

/// log level properties
typedef struct log_level_prop
{
    const char  *name;          ///< log level name
    const char  *print;         ///< log level name for printing
    const char  *color;         ///< log level color
} log_level_st;

/// log levels
static const log_level_st levels[] =
{
    // *INDENT-OFF*
    [LOG_OFF]   = { "off",      "OFF",      ""                      },
    [LOG_CRIT]  = { "critical", "CRIT",     COLOR_BRIGHT_RED        },
    [LOG_ERROR] = { "error",    "ERROR",    COLOR_BRIGHT_RED        },
    [LOG_WARN]  = { "warning",  "WARN",     COLOR_BRIGHT_YELLOW     },
    [LOG_NOTE]  = { "notice",   "NOTE",     COLOR_BRIGHT_MAGENTA    },
    [LOG_INFO]  = { "info",     "INFO",     COLOR_BRIGHT_GREEN      },
    [LOG_DEBUG] = { "debug",    "DEBUG",    COLOR_BRIGHT_WHITE      },
    [LOG_TRACE] = { "trace",    "TRACE",    COLOR_BRIGHT_BLACK      }
    // *INDENT-ON*
};

/// log error type definition
ERROR_DEFINE_LIST(LOG,
    ERROR_INFO(E_LOG_CALLBACK,       "Callback error."),
    ERROR_INFO(E_LOG_EXISTS,         "Log unit exists already."),
    ERROR_INFO(E_LOG_FOPEN,          "fopen error."),
    ERROR_INFO(E_LOG_INVALID_NAME,   "Invalid unit or target name."),
    ERROR_INFO(E_LOG_INVALID_STREAM, "Invalid stream."),
    ERROR_INFO(E_LOG_NOT_FOUND,      "Log unit/target/level unknown.")
);

/// default error type for log module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_LOG


/// Vector dtor callback for freeing log unit.
///
/// \implements vec_dtor_cb
static void log_vec_free_unit(vec_const_ct vec, void *elem, void *ctx)
{
    log_unit_st *unit = elem;

    str_unref(unit->name);

    if(unit->sinks)
        vec_free(unit->sinks);
}

/// Vector dtor callback for freeing log target.
///
/// \implements vec_dtor_cb
static void log_vec_free_target(vec_const_ct vec, void *elem, void *ctx)
{
    log_target_st *target = elem;

    str_unref(target->name);

    fflush(target->stream);

    if(target->close)
        fclose(target->stream);
}

void log_free(void)
{
    if(log.units)
        vec_free_f(log.units, log_vec_free_unit, NULL);

    if(log.targets)
        vec_free_f(log.targets, log_vec_free_target, NULL);

    if(log.prefix)
        str_unref(log.prefix);

    if(log.specs)
        vec_free(log.specs);

    memset(&log, 0, sizeof(log_st));
}

/// log find state
typedef struct log_find_state
{
    str_const_ct    name;   ///< unit/target name
    bool            exact;  ///< exact or prefix search
} log_find_st;

/// Vector predicate callback for finding log unit.
///
/// \implements vec_pred_cb
static bool log_vec_find_unit(vec_const_ct vec, const void *elem, void *ctx)
{
    const log_unit_st *unit = elem;
    log_find_st *state      = ctx;

    if(state->exact)
        return !str_cmp(unit->name, state->name);
    else
        return !str_cmp_n(unit->name, state->name, str_len(state->name));
}

ssize_t log_unit_add(str_const_ct name)
{
    log_find_st state = { .name = name, .exact = true };
    log_unit_st *unit;

    assert(name);
    return_error_if_pass(str_is_empty(name), E_LOG_INVALID_NAME, -1);

    if(log.units && vec_find(log.units, log_vec_find_unit, &state))
        return error_set(E_LOG_EXISTS), -1;

    if(!log.units && !(log.units = vec_new_c(2, sizeof(log_unit_st))))
        return error_wrap(), -1;

    if(!(unit = vec_push(log.units)))
        return error_wrap(), -1;

    if(!(unit->name = str_ref(name)))
        return error_wrap(), vec_pop(log.units), -1;

    return vec_size(log.units);
}

ssize_t log_unit_get(str_const_ct name, bool exact)
{
    log_find_st state = { .name = name, .exact = exact };
    ssize_t unit;

    assert(name);

    if(!log.units || (unit = vec_find_pos(log.units, log_vec_find_unit, &state)) < 0)
        return error_set(E_LOG_NOT_FOUND), -1;

    return unit + 1;
}

str_const_ct log_unit_get_name(size_t unit)
{
    log_unit_st *log_unit;

    if(!unit || !log.units || !(log_unit = vec_at(log.units, unit - 1)))
        return error_set(E_LOG_NOT_FOUND), NULL;

    return log_unit->name;
}

log_level_id log_unit_get_max_level(size_t unit)
{
    log_unit_st *log_unit;

    if(!unit || !log.units || !(log_unit = vec_at(log.units, unit - 1)))
        return error_set(E_LOG_NOT_FOUND), LOG_INVALID;

    return log_unit->level;
}

/// log fold state
typedef struct log_fold_state
{
    log_fold_cb fold;   ///< callback to invoke on each unit/target
    void        *ctx;   ///< callback context
} log_fold_st;

/// Vector fold callback for folding log unit.
///
/// \implements vec_fold_cb
///
/// \retval 0                   success
/// \retval <0/E_LOG_CALLBACK   \p fold error
/// \retval >0                  \p fold rc
static int log_vec_fold_unit(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    log_fold_st *state  = ctx;
    log_unit_st *unit   = elem;

    return error_pack_int(E_LOG_CALLBACK,
        state->fold(index + 1, unit->name, state->ctx));
}

int log_unit_fold(log_fold_cb fold, const void *ctx)
{
    log_fold_st state = { .fold = fold, .ctx = (void *)ctx };

    assert(fold);

    if(!log.units)
        return 0;

    return error_pick_int(E_VEC_CALLBACK,
        vec_fold(log.units, log_vec_fold_unit, &state));
}

size_t log_units(void)
{
    return log.units ? vec_size(log.units) : 0;
}

ssize_t log_target_add_file(str_const_ct name, str_const_ct file, bool append, log_color_id color)
{
    FILE *stream;
    ssize_t target;

    assert(file);

    if(!(stream = fopen(str_c(file), append ? "ab" : "wb")))
        return error_pack_last_errno(E_LOG_FOPEN, fopen), -1;

    if((target = log_target_add_stream(name ? name : file, stream, true, color)) < 0)
        return error_pass(), fclose(stream), -1;

    return target;
}

ssize_t log_target_add_stream(str_const_ct name, FILE *stream, bool close, log_color_id color)
{
    log_target_st *target;
    int fd;

    assert(name);
    assert(stream);
    assert(color < LOG_COLOR_MODES);
    return_error_if_pass(str_is_empty(name), E_LOG_INVALID_NAME, -1);

    if((fd = fileno(stream)) < 0)
        return error_pack_last_errno(E_LOG_INVALID_STREAM, fileno), -1;

    if(!log.targets && !(log.targets = vec_new_c(2, sizeof(log_target_st))))
        return error_wrap(), -1;

    if(!(target = vec_push(log.targets)))
        return error_wrap(), -1;

    if(!(target->name = str_ref(name)))
        return error_wrap(), vec_pop(log.targets), -1;

    target->stream  = stream;
    target->close   = close;

    if(color == LOG_COLOR_AUTO)
        target->color = !!isatty(fd);
    else
        target->color = color == LOG_COLOR_ON;

    return vec_size(log.targets);
}

ssize_t log_target_add_stdout(log_color_id color)
{
    return log_target_add_stream(LIT("stdout"), stdout, false, color);
}

ssize_t log_target_add_stderr(log_color_id color)
{
    return log_target_add_stream(LIT("stderr"), stderr, false, color);
}

/// Vector predicate callback for finding log target.
///
/// \implements vec_pred_cb
static bool log_vec_find_target(vec_const_ct vec, const void *elem, void *ctx)
{
    const log_target_st *target = elem;
    log_find_st *state          = ctx;

    if(state->exact)
        return !str_cmp(target->name, state->name);
    else
        return !str_cmp_n(target->name, state->name, str_len(state->name));
}

ssize_t log_target_get(str_const_ct name, bool exact)
{
    log_find_st state = { .name = name, .exact = exact };
    ssize_t target;

    assert(name);

    if(!log.targets || (target = vec_find_pos(log.targets, log_vec_find_target, &state)) < 0)
        return error_set(E_LOG_NOT_FOUND), -1;

    return target + 1;
}

str_const_ct log_target_get_name(size_t target)
{
    log_target_st *log_target;

    if(!target || !log.targets || !(log_target = vec_at(log.targets, target - 1)))
        return error_set(E_LOG_NOT_FOUND), NULL;

    return log_target->name;
}

/// Vector predicate callback for finding log sink by target.
///
/// \implements vec_pred_cb
static bool log_vec_find_sink(vec_const_ct vec, const void *elem, void *ctx)
{
    const log_target_st *target = ctx;
    const log_sink_st *sink     = elem;

    return sink->target == target;
}

/// Vector fold callback for removing log sink.
///
/// \implements vec_fold_cb
///
/// \retval 0   always success
static int log_vec_remove_sink(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    log_target_st *target   = ctx;
    log_unit_st *unit       = elem;

    if(unit->sinks)
        vec_find_remove(unit->sinks, log_vec_find_sink, target);

    return 0;
}

int log_target_remove(size_t target)
{
    log_target_st *log_target;

    if(!target || !log.targets || !(log_target = vec_at(log.targets, target - 1)))
        return error_set(E_LOG_NOT_FOUND), -1;

    if(log.units)
        vec_fold(log.units, log_vec_remove_sink, log_target);

    vec_remove_f(log.targets, log_target, log_vec_free_target, NULL);

    return 0;
}

int log_target_set_hook(size_t target, log_hook_cb hook, const void *ctx)
{
    log_target_st *log_target;

    if(!target || !log.targets || !(log_target = vec_at(log.targets, target - 1)))
        return error_set(E_LOG_NOT_FOUND), -1;

    log_target->hook    = hook;
    log_target->ctx     = (void *)ctx;

    return 0;
}

/// Vector fold callback for folding log target.
///
/// \implements vec_fold_cb
///
/// \retval 0                   success
/// \retval <0/E_LOG_CALLBACK   \p fold error
/// \retval >0                  \p fold rc
static int log_vec_fold_target(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    log_fold_st *state      = ctx;
    log_target_st *target   = elem;

    return error_pack_int(E_LOG_CALLBACK,
        state->fold(index + 1, target->name, state->ctx));
}

int log_target_fold(log_fold_cb fold, const void *ctx)
{
    log_fold_st state = { .fold = fold, .ctx = (void *)ctx };

    assert(fold);

    if(!log.targets)
        return 0;

    return error_pick_int(E_VEC_CALLBACK,
        vec_fold(log.targets, log_vec_fold_target, &state));
}

size_t log_targets(void)
{
    return log.targets ? vec_size(log.targets) : 0;
}

/// Vector fold callback for finding max sink log level.
///
/// \implements vec_fold_cb
///
/// \retval 0   always success
static int log_vec_max_level(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    log_level_id *level = ctx;
    log_sink_st *sink   = elem;

    *level = MAX(*level, sink->level);

    return 0;
}

/// Get max log level of sinks.
///
/// \param sinks    sink list
///
/// \returns        log level
static log_level_id log_sinks_max_level(vec_const_ct sinks)
{
    log_level_id level = LOG_OFF;

    vec_fold(sinks, log_vec_max_level, &level);

    return level;
}

/// Add, set or remove log unit sink for target.
///
/// \param unit     log unit
/// \param target   log target
/// \param level    log level
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int log_sink_set(log_unit_st *unit, log_target_st *target, log_level_id level)
{
    log_sink_st *sink;

    if(level == LOG_OFF)
    {
        if(unit->sinks && !vec_find_remove(unit->sinks, log_vec_find_sink, target))
        {
            unit->sinks = vec_free_if_empty(unit->sinks);
            unit->level = unit->sinks ? log_sinks_max_level(unit->sinks) : LOG_OFF;
        }
    }
    else if(!unit->sinks || !(sink = vec_find(unit->sinks, log_vec_find_sink, target)))
    {
        if(!unit->sinks && !(unit->sinks = vec_new_c(2, sizeof(log_sink_st))))
            return error_wrap(), -1;

        if(!(sink = vec_push(unit->sinks)))
            return error_wrap(), -1;

        sink->target    = target;
        sink->level     = level;
        unit->level     = MAX(unit->level, sink->level);
    }
    else
    {
        sink->target    = target;
        sink->level     = level;
        unit->level     = sink->level >= unit->level
                          ? sink->level : log_sinks_max_level(unit->sinks);
    }

    return 0;
}

/// log sink2 state
typedef struct log_sink2_state
{
    log_unit_st     *unit;  ///< log unit
    log_level_id    level;  ///< log level
} log_sink2_st;

/// Vector fold callback for setting sink for log unit.
///
/// \implements vec_fold_cb
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int log_vec_set_sink(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    log_sink2_st *state     = ctx;
    log_target_st *target   = elem;

    return error_pass_int(log_sink_set(state->unit, target, state->level));
}

/// Add or set log unit sinks for all target(s).
///
/// \param unit     log unit
/// \param target   log target ID, may be LOG_ALL_TARGETS to set all targets
/// \param level    log level
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  target not found
/// \retval -1/E_GENERIC_OOM    out of memory
static int log_sinks_set(log_unit_st *unit, size_t target, log_level_id level)
{
    log_sink2_st state = { .unit = unit, .level = level };
    log_target_st *log_target;

    if(target == LOG_ALL_TARGETS)
    {
        return error_pick_int(E_VEC_CALLBACK,
            vec_fold(log.targets, log_vec_set_sink, &state));
    }

    if(!(log_target = vec_at(log.targets, target - 1)))
        return error_pack(E_LOG_NOT_FOUND), -1;

    return error_pass_int(log_sink_set(unit, log_target, level));
}

/// log sink1 state
typedef struct log_sink1_state
{
    size_t          target;     ///< target ID
    log_level_id    level;      ///< log level
} log_sink1_st;

/// Vector fold callback for setting sinks for log unit.
///
/// \implements vec_fold_cb
///
/// \retval 0                   success
/// \retval -1/E_LOG_NOT_FOUND  target not found
/// \retval -1/E_GENERIC_OOM    out of memory
static int log_vec_set_sinks(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    log_sink1_st *state = ctx;
    log_unit_st *unit   = elem;

    return error_pass_int(log_sinks_set(unit, state->target, state->level));
}

int log_sink_set_level(size_t unit, size_t target, log_level_id level)
{
    log_sink1_st state = { .target = target, .level = level };
    log_unit_st *log_unit;

    assert(level && level < LOG_LEVELS);
    return_error_if_fail(log.units && log.targets, E_LOG_NOT_FOUND, -1);

    if(unit == LOG_ALL_UNITS)
    {
        return error_pick_int(E_VEC_CALLBACK,
            vec_fold(log.units, log_vec_set_sinks, &state));
    }

    if(!(log_unit = vec_at(log.units, unit - 1)))
        return error_pack(E_LOG_NOT_FOUND), -1;

    return error_pass_int(log_sinks_set(log_unit, target, level));
}

log_level_id log_sink_get_level(size_t unit, size_t target)
{
    log_unit_st *log_unit;
    log_target_st *log_target;
    log_sink_st *sink;

    if(!unit || !log.units || !(log_unit = vec_at(log.units, unit - 1)))
        return error_set(E_LOG_NOT_FOUND), LOG_INVALID;

    if(!target || !log.targets || !(log_target = vec_at(log.targets, target - 1)))
        return error_set(E_LOG_NOT_FOUND), LOG_INVALID;

    if(!log_unit->sinks)
        return LOG_OFF;

    sink = vec_find(log_unit->sinks, log_vec_find_sink, log_target);

    return sink ? sink->level : LOG_OFF;
}

/// log sink fold state
typedef struct log_sink_fold_state
{
    log_unit_st         *unit;  ///< log unit
    log_sink_fold_cb    fold;   ///< callback to invoke on each sink
    void                *ctx;   ///< callback context
} log_sink_fold_st;

/// Vector fold callback for folding log sink.
///
/// \implements vec_fold_cb
///
/// \retval 0                   success
/// \retval <0/E_LOG_CALLBACK   \p fold error
/// \retval >0                  \p fold rc
static int log_vec_fold_sink(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    log_sink_fold_st *state = ctx;
    log_sink_st *sink       = elem;
    size_t unit             = vec_pos(log.units, state->unit) + 1;
    size_t target           = vec_pos(log.targets, sink->target) + 1;

    return error_pack_int(E_LOG_CALLBACK,
        state->fold(unit, state->unit->name, target, sink->target->name, sink->level, state->ctx));
}

int log_sink_fold(size_t unit, log_sink_fold_cb fold, const void *ctx)
{
    log_sink_fold_st state = { .fold = fold, .ctx = (void *)ctx };

    assert(fold);

    if(!unit || !log.units || !(state.unit = vec_at(log.units, unit - 1)))
        return error_set(E_LOG_NOT_FOUND), -1;

    if(!state.unit->sinks)
        return 0;

    return error_pick_int(E_VEC_CALLBACK,
        vec_fold(state.unit->sinks, log_vec_fold_sink, &state));
}

ssize_t log_sinks(size_t unit)
{
    log_unit_st *log_unit;

    if(!unit || !log.units || !(log_unit = vec_at(log.units, unit - 1)))
        return error_set(E_LOG_NOT_FOUND), -1;

    return log_unit->sinks ? vec_size(log_unit->sinks) : 0;
}

log_level_id log_level_get(str_const_ct name, bool exact)
{
    log_level_id level;
    int rc;

    assert(name);

    for(level = LOG_OFF; level < LOG_LEVELS; level++)
    {
        if(exact)
            rc = str_cmp_c(name, levels[level].name);
        else
            rc = str_cmp_nc(name, levels[level].name, str_len(name));

        if(!rc)
            return level;
    }

    return_error_if_reached(E_LOG_NOT_FOUND, LOG_INVALID);
}

const char *log_level_get_name(log_level_id level)
{
    assert(level && level < LOG_LEVELS);

    return levels[level].name;
}

/// Vector fold callback for flushing log target.
///
/// \implements vec_map_cb
///
/// \retval 0   always success
static int log_vec_flush_target(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    log_target_st *target = elem;

    fflush(target->stream);

    return 0;
}

void log_flush(void)
{
    if(log.targets)
        vec_fold(log.targets, log_vec_flush_target, NULL);
}

int log_prefix_set(str_const_ct prefix)
{
    if(prefix && !(prefix = str_ref(prefix)))
        return error_wrap(), -1;

    if(log.prefix)
        str_unref(log.prefix);

    log.prefix = prefix;

    return 0;
}

int log_prefix_add_spec(char spec, log_spec_cb write, const void *ctx)
{
    log_spec_st *log_spec;

    assert(write);

    if(!log.specs && !(log.specs = vec_new_c(2, sizeof(log_spec_st))))
        return error_wrap(), -1;

    if(!(log_spec = vec_push(log.specs)))
        return error_wrap(), -1;

    log_spec->spec  = spec;
    log_spec->write = write;
    log_spec->ctx   = (void *)ctx;

    return 0;
}

/// Vector find callback for finding log message prefix custom specifier.
///
/// \implements vec_pred_cb
static bool log_vec_find_spec(vec_const_ct vec, const void *elem, void *ctx)
{
    const log_spec_st *spec = elem;
    const char *spec_char   = ctx;

    return spec->spec == spec_char[0];
}

/// Write prefix to target.
///
/// \param unit     unit
/// \param level    log level
/// \param target   target
static void log_write_prefix(const log_unit_st *unit, log_level_id level, const log_target_st *target)
{
    const char *prefix, *spec;
    log_spec_st *custom;
    tm_st *tm = NULL;
    time_t now;
    char *end;
    int width;

    for(prefix = str_c(log.prefix); (spec = strchr(prefix, '^')); prefix = spec + 1)
    {
        if(spec > prefix)
            fprintf(target->stream, "%.*s", (int)(spec - prefix), prefix);

        width   = strtol(spec + 1, &end, 0);
        spec    = end;

        if(!spec[0]) // missing type, ignore
            return;

        switch(spec[0])
        {
        case '^':
            fprintf(target->stream, "%*s", width, "^");
            break;

        case 'c':

            if(target->color)
                fprintf(target->stream, "%s", levels[level].color);

            break;

        case 'l':
            fprintf(target->stream, "%*s", width, levels[level].print);
            break;

        case 'p':
            fprintf(target->stream, "%*ld", width, (long)getpid());
            break;

        case 'r':

            if(target->color)
                fprintf(target->stream, "%s", COLOR_OFF);

            break;

        case 't':
            fprintf(target->stream, "%*s", width, str_c(target->name));
            break;

        case 'u':
            fprintf(target->stream, "%*s", width, str_c(unit->name));
            break;

        case 'D':

            if(!tm)
            {
                now = time(NULL);
                tm  = localtime(&now);
            }

            fprintf(target->stream, "%*s", width, time_isodate(tm));

            break;

        case 'T':

            if(!tm)
            {
                now = time(NULL);
                tm  = localtime(&now);
            }

            fprintf(target->stream, "%*s", width, time_isotime(tm));

            break;

        default:

            if(log.specs && (custom = vec_find(log.specs, log_vec_find_spec, spec)))
                custom->write(target->stream, width, custom->ctx);
            else // unknown type, just print verbatim
                fprintf(target->stream, "%*c", width, spec[0]);
        }
    }

    fprintf(target->stream, "%s", prefix);
}

/// log message type
typedef enum log_msg_type
{
    LOG_MSG_FIRST   = 1,    ///< first message to write to target
    LOG_MSG_LAST    = 2,    ///< last message to write to target
} log_msg_fs;

/// log message
typedef struct log_msg_state
{
    const log_unit_st   *unit;  ///< log unit
    log_level_id        level;  ///< log level
    log_msg_fs          type;   ///< message type
    const char          *fmt;   ///< format message
    va_list             ap;     ///< format message args
} log_msg_st;

/// Vector fold callback for writing message to target.
///
/// \implements vec_fold_cb
///
/// \retval 0   always success
static int log_vec_write_msg(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    log_msg_st *msg         = ctx;
    log_sink_st *sink       = elem;
    log_target_st *target   = sink->target;
    va_list ap;

    if(msg->level > sink->level)
        return 0;

    if(msg->type & LOG_MSG_FIRST)
    {
        if(target->hook)
            target->hook(vec_pos(log.targets, target) + 1, target->name, true, target->ctx);

        if(log.prefix)
            log_write_prefix(msg->unit, msg->level, target);
    }

    va_copy(ap, msg->ap);
    vfprintf(target->stream, msg->fmt, ap);
    va_end(ap);

    if(msg->type & LOG_MSG_LAST)
    {
        fprintf(target->stream, "%s\n", target->color ? COLOR_OFF : "");

        if(target->hook)
            target->hook(vec_pos(log.targets, target) + 1, target->name, false, target->ctx);
    }

    return 0;
}

/// Write log message to all log unit sinks.
///
/// \param unit     unit
/// \param level    log level
/// \param type     message type
/// \param fmt      format message
/// \param ...      \p fmt args
static void log_write(const log_unit_st *unit, log_level_id level, log_msg_fs type, const char *fmt, ...)
{
    log_msg_st msg = { .unit = unit, .level = level, .type = type, .fmt = fmt };

    va_start(msg.ap, fmt);
    vec_fold(unit->sinks, log_vec_write_msg, &msg);
    va_end(msg.ap);
}

/// Write log message to all log unit sinks.
///
/// \param unit     unit
/// \param level    log level
/// \param type     message type
/// \param fmt      format message
/// \param ap       \p fmt args
static void log_write_v(const log_unit_st *unit, log_level_id level, log_msg_fs type, const char *fmt, va_list ap)
{
    log_msg_st msg = { .unit = unit, .level = level, .type = type, .fmt = fmt };

    va_copy(msg.ap, ap);
    vec_fold(unit->sinks, log_vec_write_msg, &msg);
    va_end(msg.ap);
}

int log_msg(size_t unit, log_level_id level, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_v(unit, level, msg, ap);
    va_end(ap);

    return rc;
}

int log_msg_v(size_t unit, log_level_id level, const char *msg, va_list ap)
{
    log_unit_st *log_unit;

    assert(level && level < LOG_LEVELS);
    assert(msg);

    if(!unit || !log.units || !(log_unit = vec_at(log.units, unit - 1)))
        return error_set(E_LOG_NOT_FOUND), -1;

    if(level == LOG_OFF || level > log_unit->level)
        return 0;

    log_write_v(log_unit, level, LOG_MSG_FIRST | LOG_MSG_LAST, msg, ap);

    return 0;
}

int log_msg_e(size_t unit, log_level_id level, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_ev(unit, level, msg, ap);
    va_end(ap);

    return rc;
}

int log_msg_ev(size_t unit, log_level_id level, const char *msg, va_list ap)
{
    log_unit_st *log_unit;

    assert(level && level < LOG_LEVELS);
    assert(msg);

    if(!unit || !log.units || !(log_unit = vec_at(log.units, unit - 1)))
        return error_set(E_LOG_NOT_FOUND), -1;

    if(level == LOG_OFF || level > log_unit->level)
        return 0;

    log_write_v(log_unit, level, LOG_MSG_FIRST, msg, ap);
    log_write(log_unit, level, LOG_MSG_LAST, ": %s", error_desc(0));

    return 0;
}

int log_crit(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_v(unit, LOG_CRIT, msg, ap);
    va_end(ap);

    return rc;
}

int log_crit_e(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_ev(unit, LOG_CRIT, msg, ap);
    va_end(ap);

    return rc;
}

int log_error(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_v(unit, LOG_ERROR, msg, ap);
    va_end(ap);

    return rc;
}

int log_error_e(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_ev(unit, LOG_ERROR, msg, ap);
    va_end(ap);

    return rc;
}

int log_warn(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_v(unit, LOG_WARN, msg, ap);
    va_end(ap);

    return rc;
}

int log_warn_e(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_ev(unit, LOG_WARN, msg, ap);
    va_end(ap);

    return rc;
}

int log_note(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_v(unit, LOG_NOTE, msg, ap);
    va_end(ap);

    return rc;
}

int log_note_e(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_ev(unit, LOG_NOTE, msg, ap);
    va_end(ap);

    return rc;
}

int log_info(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_v(unit, LOG_INFO, msg, ap);
    va_end(ap);

    return rc;
}

int log_info_e(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_ev(unit, LOG_INFO, msg, ap);
    va_end(ap);

    return rc;
}

int log_debug(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_v(unit, LOG_DEBUG, msg, ap);
    va_end(ap);

    return rc;
}

int log_debug_e(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_ev(unit, LOG_DEBUG, msg, ap);
    va_end(ap);

    return rc;
}

int log_trace(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_v(unit, LOG_TRACE, msg, ap);
    va_end(ap);

    return rc;
}

int log_trace_e(size_t unit, const char *msg, ...)
{
    va_list ap;
    int rc;

    va_start(ap, msg);
    rc = log_msg_ev(unit, LOG_TRACE, msg, ap);
    va_end(ap);

    return rc;
}
