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

#include <ytil/sys/path.h>
#include <ytil/sys/env.h>
#include <ytil/def.h>

#ifdef _WIN32
#   include <ShlObj.h>
#else
#   include <unistd.h>
#   include <pwd.h>
#endif

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_PATH_NOT_AVAILABLE, "Path not available.")
};

/*
typedef struct path_user_dir_xdg_info
{
    const char *env, *def;
} path_user_dir_xdg_info_st;

static const path_user_dir_xdg_info_st path_user_dir_xdg_infos[] =
{
      [PATH_USER_DIR_HOME]        = { "HOME",                NULL }
    , [PATH_USER_DIR_DESKTOP]     = { "XDG_DESKTOP_DIR",     "Desktop" }
    , [PATH_USER_DIR_DOCUMENTS]   = { "XDG_DOCUMENTS_DIR",   "Documents" }
    , [PATH_USER_DIR_DOWNLOAD]    = { "XDG_DOWNLOAD_DIR",    "Downloads" }
    , [PATH_USER_DIR_MUSIC]       = { "XDG_MUSIC_DIR",       "Music" }
    , [PATH_USER_DIR_PICTURES]    = { "XDG_PICTURES_DIR",    "Pictures" }
    , [PATH_USER_DIR_PUBLICSHARE] = { "XDG_PUBLICSHARE_DIR", "Public" }
    , [PATH_USER_DIR_TEMPLATES]   = { "XDG_TEMPLATES_DIR",   "Templates" }
    , [PATH_USER_DIR_VIDEOS]      = { "XDG_VIDEOS_DIR",      "Videos" }
};*/

path_ct path_get_user_dir(path_user_dir_id id)
{
    str_const_ct value;
    path_ct path;
    
    assert(id < PATH_USER_DIRS);
    
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

typedef struct path_app_dir_xdg_info
{
    const char *env, *def, *sub, *sub_def;
} path_app_dir_xdg_info_st;

static const path_app_dir_xdg_info_st path_app_dir_xdg_infos[] =
{
      [PATH_APP_DIR_CACHE]   = { "XDG_CACHE_HOME",  ".cache",       NULL,   NULL }
    , [PATH_APP_DIR_CONFIG]  = { "XDG_CONFIG_HOME", ".config",      NULL,   NULL }
    , [PATH_APP_DIR_DATA]    = { "XDG_DATA_HOME",   ".local/share", NULL,   NULL }
    , [PATH_APP_DIR_LOG]     = { "XDG_CACHE_HOME",  ".cache",       "logs", "logs" }
    , [PATH_APP_DIR_RUNTIME] = { "XDG_RUNTIME_DIR", ".cache",       NULL,   "run" }
};

static path_ct path_get_app_dir_xdg(path_app_dir_id id, str_const_ct app, str_const_ct version)
{
    const path_app_dir_xdg_info_st *info = &path_app_dir_xdg_infos[id];
    str_const_ct value;
    path_ct path;
    
    if(!(value = env_get(STR(info->env))))
        return error_set(E_PATH_NOT_AVAILABLE), NULL;
    
    if(!(path = path_new(value, PATH_STYLE_NATIVE)))
        return error_wrap(), NULL;
    
    if(!path_append(path, app, PATH_STYLE_NATIVE)
    || (version && !path_append(path, version, PATH_STYLE_NATIVE))
    || (info->sub && !path_append_c(path, info->sub, PATH_STYLE_POSIX)))
        return error_wrap(), path_free(path), NULL;
    
    return path;
}

static path_ct path_get_app_dir_xdg_default(path_app_dir_id id, str_const_ct app, str_const_ct version)
{
    const path_app_dir_xdg_info_st *info = &path_app_dir_xdg_infos[id];
    path_ct path;
    
    if(!(path = path_get_user_dir(PATH_USER_DIR_HOME)))
        return error_pass(), NULL;
    
    if(!path_append_c(path, info->def, PATH_STYLE_POSIX)
    || !path_append(path, app, PATH_STYLE_NATIVE)
    || (version && !path_append(path, version, PATH_STYLE_NATIVE))
    || (info->sub_def && !path_append_c(path, info->sub_def, PATH_STYLE_POSIX)))
        return error_wrap(), path_free(path), NULL;
    
    return path;
}

typedef struct path_app_dir_windows_info
{
    const char *env;
    int csidl;
    const char *sub;
} path_app_dir_windows_info_st;

#ifdef _WIN32
#   define CSIDL(id) CSIDL_##id
#else
#   define CSIDL(id) 0
#endif

static const path_app_dir_windows_info_st path_app_dir_windows_infos[] =
{
      [PATH_APP_DIR_CACHE]   = { "LOCALAPPDATA", CSIDL(LOCAL_APPDATA), "cache" }
    , [PATH_APP_DIR_CONFIG]  = { "APPDATA",      CSIDL(APPDATA),       NULL }
    , [PATH_APP_DIR_DATA]    = { "LOCALAPPDATA", CSIDL(LOCAL_APPDATA), NULL }
    , [PATH_APP_DIR_LOG]     = { "LOCALAPPDATA", CSIDL(LOCAL_APPDATA), "logs" }
    , [PATH_APP_DIR_RUNTIME] = { "LOCALAPPDATA", CSIDL(LOCAL_APPDATA), "run" }
};

static path_ct path_get_app_dir_windows(path_app_dir_id id, str_const_ct author, str_const_ct app, str_const_ct version)
{
    const path_app_dir_windows_info_st *info = &path_app_dir_windows_infos[id];
    str_const_ct value;
    path_ct path;
    
    if(!(value = env_get(STR(info->env))))
    {
#ifndef _WIN32
        return error_set(E_PATH_NOT_AVAILABLE), NULL;
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

path_ct path_get_app_dir(path_app_dir_id id, str_const_ct author, str_const_ct app, str_const_ct version)
{
    path_ct path;
    
    assert(id < PATH_APP_DIRS && author && app);
    
    if((path = path_get_app_dir_xdg(id, app, version)))
        return path;
    
    if(!error_check(0, E_PATH_NOT_AVAILABLE))
        return error_pass(), NULL;
    
    if((path = path_get_app_dir_windows(id, author, app, version)))
        return path;
    
    if(!error_check(0, E_PATH_NOT_AVAILABLE))
        return error_pass(), NULL;
    
    return error_pass_ptr(path_get_app_dir_xdg_default(id, app, version));
}
