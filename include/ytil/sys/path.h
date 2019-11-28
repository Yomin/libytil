/*
 * Copyright (c) 2019 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef __YTIL_SYS_PATH_H__
#define __YTIL_SYS_PATH_H__

#include <ytil/gen/path.h>

typedef enum sys_path_error
{
      E_PATH_INVALID_APP_AUTHOR
    , E_PATH_INVALID_APP_NAME
    , E_PATH_INVALID_APP_VERSION
    , E_PATH_NOT_AVAILABLE
} sys_path_error_id;

typedef enum path_sys_dir
{
      PATH_SYS_DIR_HOME
    , PATH_SYS_DIR_TMP
    , PATH_SYS_DIRS
} path_sys_dir_id;

typedef enum path_user_dir
{
      PATH_USER_DIR_DESKTOP
    , PATH_USER_DIR_DOCUMENTS
    , PATH_USER_DIR_DOWNLOADS
    , PATH_USER_DIR_MUSIC
    , PATH_USER_DIR_PICTURES
    , PATH_USER_DIR_PUBLIC
    , PATH_USER_DIR_TEMPLATES
    , PATH_USER_DIR_VIDEOS
    , PATH_USER_DIRS
} path_user_dir_id;

typedef enum path_app_dir
{
      PATH_APP_DIR_CACHE
    , PATH_APP_DIR_CONFIG
    , PATH_APP_DIR_DATA
    , PATH_APP_DIR_LOG
    , PATH_APP_DIR_RUNTIME
    , PATH_APP_DIR_TMP
    , PATH_APP_DIRS
} path_app_dir_id;


// get system directory
path_ct path_get_sys_dir(path_sys_dir_id id);
// get user directory
path_ct path_get_user_dir(path_user_dir_id id);
// get application directory, author/app/version may be NULL
path_ct path_get_app_dir(path_app_dir_id id, str_const_ct author, str_const_ct app, str_const_ct version);

#endif
