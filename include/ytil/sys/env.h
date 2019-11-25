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

#ifndef __YTIL_SYS_ENV_H__
#define __YTIL_SYS_ENV_H__

#include <ytil/gen/str.h>
#include <ytil/gen/path.h>

typedef enum env_error
{
      E_ENV_INVALID_NAME
    , E_ENV_NOT_AVAILABLE
    , E_ENV_NOT_FOUND
} env_error_id;

typedef enum env_app_dir
{
      ENV_APP_DIR_CACHE
    , ENV_APP_DIR_CONFIG
    , ENV_APP_DIR_DATA
    , ENV_APP_DIR_LOG
    , ENV_APP_DIR_VOLATILE
    , ENV_APP_DIRS
} env_app_dir_id;

// return 0 to continue fold, anything else stops fold
typedef int (*env_fold_cb)(str_const_ct name, str_const_ct value, void *ctx);


// initialize env from environ
// this is implicitly called by most env functions if not initialized yet
int  env_init(void);
// free env
void env_free(void);

// add/overwrite environment variable 'name' with 'value'
int          env_set(str_const_ct name, str_const_ct value);
// retrieve environment variable 'name'
str_const_ct env_get(str_const_ct name);
// reset/unset environment variable 'name' to default/nothing
int          env_reset(str_const_ct name);
// unset environment variable 'name'
int          env_unset(str_const_ct name);
// apply fold to each environment variable
int          env_fold(env_fold_cb fold, void *ctx);
// dump all environment variables to stdout
void         env_dump(void);

// get environment specific user directory
path_ct env_get_home(void);
// get environment specific application directory
// version may be NULL
path_ct env_get_app_dir(env_app_dir_id id, str_const_ct author, str_const_ct app, str_const_ct version);

#endif
