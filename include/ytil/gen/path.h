/*
 * Copyright (c) 2018-2024 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_GEN_PATH_H_INCLUDED
#define YTIL_GEN_PATH_H_INCLUDED

#include <ytil/gen/str.h>
#include <ytil/gen/error.h>
#include <stdbool.h>
#include <stddef.h>


typedef enum path_error
{
    E_PATH_INVALID_DEVICE_NAME,
    E_PATH_INVALID_DRIVE,
    E_PATH_INVALID_HOST,
    E_PATH_INVALID_PATH,
    E_PATH_INVALID_SUFFIX,
    E_PATH_INVALID_SHARE,
    E_PATH_INVALID_TYPE,
    E_PATH_MALFORMED,
    E_PATH_UNSUPPORTED,
} path_error_id;

/// path error type declaration
ERROR_DECLARE(PATH);

typedef enum path_type
{
    PATH_TYPE_STANDARD,     ///< standard path
    PATH_TYPE_DRIVE,        ///< path on specific drive
    PATH_TYPE_NETWORK,      ///< UNC network path
    PATH_TYPE_DEVICE,       ///< UNC device path
    PATH_TYPES,
} path_type_id;

/// path style
typedef struct path_style
{
    const char  *current;   ///< current directory symbol
    const char  *parent;    ///< parent directory symbol
    const char  **sep;      ///< NULL terminated list of path component separators
} path_style_st;

extern const path_style_st path_style_unix;
extern const path_style_st path_style_windows;

#define PATH_STYLE_UNIX     &path_style_unix
#define PATH_STYLE_WINDOWS  &path_style_windows
#define PATH_STYLE_NATIVE   UNIX_WINDOWS(PATH_STYLE_UNIX, PATH_STYLE_WINDOWS)

struct path;
typedef struct path         *path_ct;           ///< path type
typedef const struct path   *path_const_ct;     ///< const path type


/// Create new path from str in native style.
///
/// \param str  str
///
/// \returns    new path
path_ct path_new(str_const_ct str);

/// Create new path from C string in native style.
///
/// \param str  C string
///
/// \returns    new path
path_ct path_new_c(const char *str);

/// Create new path from C string of length in native style.
///
/// \param str  C string
/// \param len  length of \p str
///
/// \returns    new path
path_ct path_new_cn(const char *str, size_t len);

/// Create new path from str.
///
/// \param str      str
/// \param style    style of \p str
///
/// \returns    new path
path_ct path_new_s(str_const_ct str, const path_style_st *style);

/// Create new path from C string.
///
/// \param str      C string
/// \param style    style of \p str
///
/// \returns    new path
path_ct path_new_cs(const char *str, const path_style_st *style);

/// Create new path from C string of length.
///
/// \param str      C string
/// \param len      length of \p str
/// \param style    style of \p str
///
/// \returns    new path
path_ct path_new_cns(const char *str, size_t len, const path_style_st *style);

/// Create new current directory path.
///
/// \returns    new path
path_ct path_new_current(void);

/// Create new parent directory path.
///
/// \returns    new path
path_ct path_new_parent(void);

// duplicate path
//path_ct path_dup(path_const_ct path);

/// Reset path to current directory.
///
/// \param path     path
void path_reset(path_ct path);

/// Free path.
///
/// \param path     path
void path_free(path_ct path);

/// Check whether path is absolute.
///
/// \param path     path
///
/// \retval true    path is absolue
/// \retval false   path is relative
bool path_is_absolute(path_const_ct path);

/// Check whether path is relative.
///
/// \param path     path
///
/// \retval true    path is relative
/// \retval false   path is absolute
bool path_is_relative(path_const_ct path);

/// Check whether path is directory (has trailing path separator).
///
/// \param path     path
///
/// \retval true    path is directory
/// \retval false   path may not be a directory
bool path_is_directory(path_const_ct path);

#define PATH_CASE_SENSITIVE     true
#define PATH_CASE_INSENSITIVE   false
#define PATH_CASE_UNIX          PATH_CASE_SENSITIVE
#define PATH_CASE_WINDOWS       PATH_CASE_INSENSITIVE
#define PATH_CASE_NATIVE        UNIX_WINDOWS(PATH_CASE_UNIX, PATH_CASE_WINDOWS)

/// Check if path1 equals path2 with native case sensitive comparison.
///
/// \param path1    path1
/// \param path2    path2
///
/// \retval true    paths are equal
/// \retval false   paths are not equal
bool path_is_equal(path_const_ct path1, path_const_ct path2);

/// Check if path1 equals path2.
///
/// \param path1            path1
/// \param path2            path2
/// \param casesensitive    compare case sensitive
///
/// \retval true    paths are equal
/// \retval false   paths are not equal
bool path_is_equal_c(path_const_ct path1, path_const_ct path2, bool casesensitive);

/// Get path type.
///
/// \param path     path
///
/// \returns        path type
path_type_id path_type(path_const_ct path);

/// Get count of path components.
///
/// \param path     path
size_t path_depth(path_const_ct path);

/// Get length of path with native style.
///
/// \param path     path
///
/// \returns        path length in bytes
size_t path_len(path_const_ct path);

/// Get length of path.
///
/// \param path     path
/// \param style    path style
///
/// \returns        path length in bytes
size_t path_len_s(path_const_ct path, const path_style_st *style);

/// Set path from str in native style.
///
/// \param path     path
/// \param str      str
///
/// \returns        path
path_ct path_set(path_ct path, str_const_ct str);

/// Set path from C string in native style.
///
/// \param path     path
/// \param str      C string
///
/// \returns        path
path_ct path_set_c(path_ct path, const char *str);

/// Set path from C string of length in native style.
///
/// \param path     path
/// \param str      C string
/// \param len      length of \p str
///
/// \returns        path
path_ct path_set_cn(path_ct path, const char *str, size_t len);

/// Set path from str.
///
/// \param path     path
/// \param str      str
/// \param style    style of \p str
///
/// \returns        path
path_ct path_set_s(path_ct path, str_const_ct str, const path_style_st *style);

/// Set path from C string.
///
/// \param path     path
/// \param str      C string
/// \param style    style of \p str
///
/// \returns        path
path_ct path_set_cs(path_ct path, const char *str, const path_style_st *style);

/// Set path from C string of length.
///
/// \param path     path
/// \param str      C string
/// \param len      length of \p str
/// \param style    style of \p str
///
/// \returns        path
path_ct path_set_cns(path_ct path, const char *str, size_t len, const path_style_st *style);

/// Set drive letter for drive paths.
///
/// \param path     path
/// \param letter   drive letter
///
/// \returns        path
path_ct path_set_drive(path_ct path, char letter);

/// Set host and share for UNC network paths.
///
/// \param path     path
/// \param host     host
/// \param share    share
///
/// \returns        path
path_ct path_set_network(path_ct path, str_const_ct host, str_const_ct share);

/// Set share for UNC network paths.
///
/// \param path     path
/// \param share    share
///
/// \returns        path
path_ct path_set_network_share(path_ct path, str_const_ct share);

/// Set port for UNC network paths.
///
/// \param path     path
/// \param port     port
///
/// \returns        path
path_ct path_set_network_port(path_ct path, int port);

/// Set SSL for UNC network paths.
///
/// \param path     path
/// \param ssl      SSL enabled
///
/// \returns        path
path_ct path_set_network_ssl(path_ct path, bool ssl);

/// Set name and ID for UNC device paths.
///
/// \param path     path
/// \param name     device name
/// \param id       device ID
///
/// \returns        path
path_ct path_set_device(path_ct path, str_const_ct name, size_t id);

/// Set ID for UNC device paths.
///
/// \param path     path
/// \param id       device ID
///
/// \returns        path
path_ct path_set_device_ident(path_ct path, size_t id);

/*
// replace suffix of last path component or add if not yet existing
path_ct path_set_suffix(path_ct path, str_const_ct suffix);
// add suffix to last path component, if a suffix already exists append suffix
path_ct path_add_suffix(path_ct path, str_const_ct suffix);
*/

/// Append str in native style as new path components.
///
/// \param path     path
/// \param str      str
///
/// \returns        path
path_ct path_append(path_ct path, str_const_ct str);

/// Append C string in native style as new path components.
///
/// \param path     path
/// \param str      C string
///
/// \returns        path
path_ct path_append_c(path_ct path, const char *str);

/// Append C string of length in native style as new path components.
///
/// \param path     path
/// \param str      C string
/// \param len      length of \p str
///
/// \returns        path
path_ct path_append_cn(path_ct path, const char *str, size_t len);

/// Append str as new path components.
///
/// \param path     path
/// \param str      str
/// \param style    style of \p str
///
/// \returns        path
path_ct path_append_s(path_ct path, str_const_ct str, const path_style_st *style);

/// Append C string as new path components.
///
/// \param path     path
/// \param str      C string
/// \param style    style of \p str
///
/// \returns        path
path_ct path_append_cs(path_ct path, const char *str, const path_style_st *style);

/// Append C string of length as new path components.
///
/// \param path     path
/// \param str      C string
/// \param len      length of \p str
/// \param style    style of \p str
///
/// \returns        path
path_ct path_append_cns(path_ct path, const char *str, size_t len, const path_style_st *style);

/// Drop n path components from end.
///
/// \param path     path
/// \param n        number of components to drop
///
/// \returns        path
path_ct path_drop(path_ct path, size_t n);

/// Remove suffix from last path components if existing.
///
/// \param path     path
///
/// \returns        path
path_ct path_drop_suffix(path_ct path);

/// Get path as str in native style.
///
/// \param path     path
///
/// \returns        native style path str
str_ct path_get(path_const_ct path);

/// Get n components of path as str in native style.
///
/// \param path     path
/// \param n        number of path components
/// \param trailing append trailing separator if path has one
///
/// \returns        native style path str
// get n path components as string
str_ct path_get_n(path_const_ct path, size_t n, bool trailing);

/// Get path as str.
///
/// \param path     path
/// \param style    path style
///
/// \returns        path str
str_ct path_get_s(path_const_ct path, const path_style_st *style);

/// Get n components of path as str.
///
/// \param path     path
/// \param n        number of path components
/// \param trailing append trailing separator if path has one
/// \param style    path style
///
/// \returns        path str
str_ct path_get_ns(path_const_ct path, size_t n, bool trailing, const path_style_st *style);

/// Get drive letter for drive paths.
///
/// \param path     path
///
/// \returns        drive letter
char path_get_drive_letter(path_const_ct path);

/// Get host name for UNC network paths.
///
/// \param path     path
///
/// \returns        host name
str_const_ct path_get_network_host(path_const_ct path);

/// Get share name for UNC network paths.
///
/// \param path     path
///
/// \returns        share name
str_const_ct path_get_network_share(path_const_ct path);

/// Get port for UNC network paths.
///
/// \param path     path
///
/// \returns        port
int path_get_network_port(path_const_ct path);

/// Get SSL enabled for UNC network paths.
///
/// \param path     path
///
/// \retval true    SSL enabled
/// \retval false   SSL disabled
bool path_get_network_ssl(path_const_ct path);

/// Get device name for UNC network paths.
///
/// \param path     path
///
/// \returns        device name
str_const_ct path_get_device_name(path_const_ct path);

/// Get device ID for UNC network paths.
///
/// \param path     path
///
/// \returns        device ID
ssize_t path_get_device_ident(path_const_ct path);

/*
// get suffix of last path component, empty str if none existing
str_ct path_get_suffix(path_const_ct path);

// get all path components but last
str_ct path_dirname(path_const_ct path, path_style_id style);
// get last path component
str_ct path_basename(path_const_ct path, path_style_id style);
*/

#endif // YTIL_GEN_PATH_H_INCLUDED
