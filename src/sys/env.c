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

#include <ytil/sys/env.h>
#include <ytil/def.h>
#include <ytil/con/art.h>
#include <stdio.h>

#ifdef _WIN32
#   include <ShlObj.h>
#else
#   include <unistd.h>
#   include <pwd.h>
#endif

typedef struct env_value
{
    const char *def;
    str_const_ct set;
} env_value_st;

typedef struct env_fold_state
{
    env_fold_cb fold;
    void *ctx;
} env_fold_state_st;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_ENV_INVALID_NAME, "Invalid environment name.")
    , ERROR_INFO(E_ENV_NOT_AVAILABLE, "Environment value not available.")
    , ERROR_INFO(E_ENV_NOT_FOUND, "Environment value not found.")
};

static art_ct env;
extern char **environ;

#define ENV_UNSET ((str_const_ct)1)


int env_init(void)
{
    env_value_st *value;
    char **var, *sep;
    str_ct name = LIT("");
    
    if(!(env = art_new(ART_MODE_UNORDERED)))
        return error_wrap(), -1;
    
    for(var = environ; *var; var++)
    {
        if(!(sep = strchr(*var, '=')) || sep == *var)
            continue;
        
        if(!(value = calloc(1, sizeof(env_value_st))))
            return error_wrap_errno(calloc), env_free(), -1;
        
        sep[0] = '\0';
        str_set_sn(name, *var, sep - *var);
        
        if(!art_insert(env, name, value))
            return error_wrap(), free(value), env_free(), sep[0] = '=', -1;
        
        sep[0] = '=';
        value->def = &sep[1];
    }
    
    return 0;
}

static void env_free_value(env_value_st *value)
{
    if(value->set && value->set != ENV_UNSET)
        str_unref(value->set);
    
    free(value);
}

static void env_art_free_value(art_const_ct art, void *data, void *ctx)
{
    env_free_value(data);
}

void env_free(void)
{
    art_free_f(env, env_art_free_value, NULL);
    env = NULL;
}

int env_set(str_const_ct name, str_const_ct str)
{
    env_value_st *value;
    
    return_error_if_pass(str_is_empty(name), E_ENV_INVALID_NAME, -1);
    
    if(!env && env_init())
        return error_pass(), -1;
    
    if(!(str = str_ref(str)))
        return error_wrap(), -1;
    
    if(!(value = art_get_data(env, name)))
    {
        if(!(value = calloc(1, sizeof(env_value_st))))
            return error_wrap_errno(calloc), str_unref(str), -1;
        
        if(!art_insert(env, name, value))
            return error_wrap(), free(value), str_unref(str), -1;
    }
    
    if(value->set && value->set != ENV_UNSET)
        str_unref(value->set);
    
    value->set = str;
    str_mark_const(str);
    
    return 0;
}

static str_const_ct env_value_get(env_value_st *value)
{
    if(!value->set && !(value->set = str_new_s(value->def)))
        return error_wrap(), NULL;
    
    return value->set;
}

str_const_ct env_get(str_const_ct name)
{
    env_value_st *value;
    
    return_error_if_pass(str_is_empty(name), E_ENV_INVALID_NAME, NULL);
    
    if(!env && env_init())
        return error_pass(), NULL;
    
    if(!(value = art_get_data(env, name)))
        return error_push(E_ENV_NOT_FOUND), NULL;
    
    if(value->set == ENV_UNSET)
        return error_set(E_ENV_NOT_FOUND), NULL;
    
    return error_pass_ptr(env_value_get(value));
}

int env_reset(str_const_ct name)
{
    art_node_ct node;
    env_value_st *value;
    
    return_error_if_pass(str_is_empty(name), E_ENV_INVALID_NAME, -1);
    
    if(!env || !(node = art_get(env, name)))
        return 0;
    
    value = art_node_get_data(node);
    
    if(!value->set)
        return 0;
    
    if(!value->def)
    {
        env_free_value(value);
        art_remove(env, node);
    }
    else if(value->set == ENV_UNSET)
    {
        value->set = NULL;
    }
    else if(str_c(value->set) != value->def)
    {
        str_unref(value->set);
        value->set = NULL;
    }
    
    return 0;
}

int env_unset(str_const_ct name)
{
    art_node_ct node;
    env_value_st *value;
    
    return_error_if_pass(str_is_empty(name), E_ENV_INVALID_NAME, -1);
    
    if(!env && env_init())
        return error_pass(), -1;
    
    if(!(node = art_get(env, name)))
        return 0;
    
    value = art_node_get_data(node);
    
    if(value->def)
    {
        if(value->set && value->set != ENV_UNSET)
            str_unref(value->set);
        
        value->set = ENV_UNSET;
    }
    else
    {
        env_free_value(value);
        art_remove(env, node);
    }
    
    return 0;
}

static int env_art_fold_value(art_const_ct art, str_const_ct key, void *data, void *ctx)
{
    env_fold_state_st *state = ctx;
    env_value_st *value = data;
    str_const_ct str;
    
    if(!(str = env_value_get(value)))
        return error_pass(), -1;
    
    if(str == ENV_UNSET)
        return 0;
    
    return error_wrap_int(state->fold(key, str, state->ctx));
}

int env_fold(env_fold_cb fold, void *ctx)
{
    env_fold_state_st state = { .fold = fold, .ctx = ctx };
    
    assert(fold);
    
    if(!env && env_init())
        return error_pass(), -1;
    
    return error_wrap_int(art_fold_k(env, env_art_fold_value, &state));
}

static int env_dump_value(str_const_ct name, str_const_ct value, void *ctx)
{
    size_t *no = ctx;
    
    printf("%03zu %s = %s\n", *no, (char*)str_bc(name), str_c(value));
    
    no[0]++;
    
    return 0;
}

void env_dump(void)
{
    size_t no = 1;
    
    env_fold(env_dump_value, &no);
}

path_ct env_get_user_dir(env_user_dir_id id)
{
    str_const_ct value;
    path_ct path;
    
    assert(id < ENV_USER_DIRS);
    
    if(!(value = env_get(LIT("HOME"))) && !(value = env_get(LIT("USERPROFILE"))))
    {
#ifdef _WIN32
        char tmp[MAX_PATH];
        HRESULT rc;
        
        if((rc = SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, tmp)) != S_OK)
            return error_wrap_hresult(SHGetFolderPath, rc), NULL;
        
        value = STR(tmp);
#else
        struct passwd *pwd;
        
        if(!(pwd = getpwuid(getuid())))
            return error_wrap_errno(getpwuid), NULL;
        
        value = STR(pwd->pw_dir);
#endif
    }
    
    if(!(path = path_new(value, PATH_STYLE_NATIVE)))
        return error_wrap(), NULL;
    
    return path;
}

typedef struct env_app_dir_xdg_info
{
    const char *env, *def, *sub, *sub_def;
} env_app_dir_xdg_info_st;

static const env_app_dir_xdg_info_st env_app_dir_xdg_infos[] =
{
      [ENV_APP_DIR_CACHE]   = { "XDG_CACHE_HOME",  ".cache",       NULL,   NULL }
    , [ENV_APP_DIR_CONFIG]  = { "XDG_CONFIG_HOME", ".config",      NULL,   NULL }
    , [ENV_APP_DIR_DATA]    = { "XDG_DATA_HOME",   ".local/share", NULL,   NULL }
    , [ENV_APP_DIR_LOG]     = { "XDG_CACHE_HOME",  ".cache",       "logs", "logs" }
    , [ENV_APP_DIR_RUNTIME] = { "XDG_RUNTIME_DIR", ".cache",       NULL,   "run" }
};

static path_ct env_get_app_dir_xdg(env_app_dir_id id, str_const_ct app, str_const_ct version)
{
    const env_app_dir_xdg_info_st *info = &env_app_dir_xdg_infos[id];
    str_const_ct value;
    path_ct path;
    
    if(!(value = env_get(STR(info->env))))
        return error_set(E_ENV_NOT_AVAILABLE), NULL;
    
    if(!(path = path_new(value, PATH_STYLE_NATIVE)))
        return error_wrap(), NULL;
    
    if(!path_append(path, app, PATH_STYLE_NATIVE)
    || (version && !path_append(path, version, PATH_STYLE_NATIVE))
    || (info->sub && !path_append_c(path, info->sub, PATH_STYLE_POSIX)))
        return error_wrap(), path_free(path), NULL;
    
    return path;
}

static path_ct env_get_app_dir_xdg_default(env_app_dir_id id, str_const_ct app, str_const_ct version)
{
    const env_app_dir_xdg_info_st *info = &env_app_dir_xdg_infos[id];
    path_ct path;
    
    if(!(path = env_get_user_dir(ENV_USER_DIR_HOME)))
        return error_pass(), NULL;
    
    if(!path_append_c(path, info->def, PATH_STYLE_POSIX)
    || !path_append(path, app, PATH_STYLE_NATIVE)
    || (version && !path_append(path, version, PATH_STYLE_NATIVE))
    || (info->sub_def && !path_append_c(path, info->sub_def, PATH_STYLE_POSIX)))
        return error_wrap(), path_free(path), NULL;
    
    return path;
}

typedef struct env_app_dir_windows_info
{
    const char *env;
    int csidl;
    const char *sub;
} env_app_dir_windows_info_st;

#ifdef _WIN32
#   define CSIDL(id) CSIDL_##id
#else
#   define CSIDL(id) 0
#endif

static const env_app_dir_windows_info_st env_app_dir_windows_infos[] =
{
      [ENV_APP_DIR_CACHE]   = { "LOCALAPPDATA", CSIDL(LOCAL_APPDATA), "cache" }
    , [ENV_APP_DIR_CONFIG]  = { "APPDATA",      CSIDL(APPDATA),       NULL }
    , [ENV_APP_DIR_DATA]    = { "LOCALAPPDATA", CSIDL(LOCAL_APPDATA), NULL }
    , [ENV_APP_DIR_LOG]     = { "LOCALAPPDATA", CSIDL(LOCAL_APPDATA), "logs" }
    , [ENV_APP_DIR_RUNTIME] = { "LOCALAPPDATA", CSIDL(LOCAL_APPDATA), "run" }
};

static path_ct env_get_app_dir_windows(env_app_dir_id id, str_const_ct author, str_const_ct app, str_const_ct version)
{
    const env_app_dir_windows_info_st *info = &env_app_dir_windows_infos[id];
    str_const_ct value;
    path_ct path;
    
    if(!(value = env_get(STR(info->env))))
    {
#ifndef _WIN32
        return error_set(E_ENV_NOT_AVAILABLE), NULL;
#else
        char tmp[MAX_PATH];
        HRESULT rc;
        
        if((rc = SHGetFolderPath(NULL, info->csidl, NULL, SHGFP_TYPE_CURRENT, tmp)) != S_OK)
            return error_wrap_hresult(SHGetFolderPath, rc), NULL;
        
        value = STR(tmp);
#endif
    }
    
    if(!(path = path_new(value, PATH_STYLE_NATIVE)))
        return error_wrap(), NULL;
    
    if(!path_append(path, author, PATH_STYLE_NATIVE)
    || !path_append(path, app, PATH_STYLE_NATIVE)
    || (version && !path_append(path, version, PATH_STYLE_NATIVE))
    || (info->sub && !path_append_c(path, info->sub, PATH_STYLE_POSIX)))
        return error_wrap(), path_free(path), NULL;
    
    return path;
}

path_ct env_get_app_dir(env_app_dir_id id, str_const_ct author, str_const_ct app, str_const_ct version)
{
    path_ct path;
    
    assert(id < ENV_APP_DIRS && author && app);
    
    if((path = env_get_app_dir_xdg(id, app, version)))
        return path;
    
    if(!error_check(0, E_ENV_NOT_AVAILABLE))
        return error_pass(), NULL;
    
    if((path = env_get_app_dir_windows(id, author, app, version)))
        return path;
    
    if(!error_check(0, E_ENV_NOT_AVAILABLE))
        return error_pass(), NULL;
    
    return error_pass_ptr(env_get_app_dir_xdg_default(id, app, version));
}
