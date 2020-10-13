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
#include <errno.h>

#ifdef _WIN32
#   include <initguid.h>
#   include <KnownFolders.h>
#   include <ShlObj.h>
#else
#   include <unistd.h>
#   include <pwd.h>
#endif

/// syspath error type definition
ERROR_DEFINE_LIST(SYSPATH,
      ERROR_INFO(E_PATH_INVALID_APP_AUTHOR, "Invalid app author.")
    , ERROR_INFO(E_PATH_INVALID_APP_NAME, "Invalid app name.")
    , ERROR_INFO(E_PATH_INVALID_APP_VERSION, "Invalid app version.")
    , ERROR_INFO(E_PATH_NOT_AVAILABLE, "Path not available.")
);

/// default error type for syspath module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_SYSPATH


#ifdef _WIN32
static path_ct path_get_win_folder(const KNOWNFOLDERID *id)
{
    wchar_t *wfolder = NULL;
    char *cfolder;
    ssize_t len;
    HRESULT rc;
    
    if((rc = SHGetKnownFolderPath(id, KF_FLAG_DEFAULT, NULL, &wfolder)) != S_OK)
        return error_wrap_hresult(SHGetKnownFolderPath, rc), NULL;
    
    if((len = wcstombs(NULL, wfolder, 0)) < 0)
        return errno = EILSEQ, error_wrap_last_errno(wcstombs), CoTaskMemFree(wfolder), NULL;
    
    cfolder = alloca(len+1);
    wcstombs(cfolder, wfolder, len+1);
    CoTaskMemFree(wfolder);
    
    return error_wrap_ptr(path_new_c(cfolder, PATH_STYLE_NATIVE));
}
#endif

static path_ct path_get_home(void)
{
    str_const_ct value;
    
    if((value = env_get(LIT("HOME"))))
        return error_wrap_ptr(path_new(value, PATH_STYLE_NATIVE));
    
    if(error_code(0) != E_ENV_NOT_FOUND)
        return error_wrap(), NULL;
    
#ifdef _WIN32
    if((value = env_get(LIT("USERPROFILE"))))
        return error_wrap_ptr(path_new(value, PATH_STYLE_NATIVE));
    
    if(error_code(0) != E_ENV_NOT_FOUND)
        return error_wrap(), NULL;
    
    return error_pass_ptr(path_get_win_folder(&FOLDERID_Profile));
#else
    struct passwd *pwd;
    
    if(!(pwd = getpwuid(getuid())))
        return error_wrap_last_errno(getpwuid), NULL;
    
    return error_wrap_ptr(path_new_c(pwd->pw_dir, PATH_STYLE_NATIVE));
#endif
}

static path_ct path_get_tmp(void)
{
    str_const_ct value;
    
    if((value = env_get(LIT("TMP")))
    || (value = env_get(LIT("TEMP")))
    || (value = env_get(LIT("TMPDIR"))))
        return error_wrap_ptr(path_new(value, PATH_STYLE_NATIVE));
    
    if(error_code(0) != E_ENV_NOT_FOUND)
        return error_wrap(), NULL;
    
#ifdef _WIN32
    return error_wrap_ptr(path_new(LIT("/windows/temp"), PATH_STYLE_NATIVE));
#else
    return error_wrap_ptr(path_new(LIT("/tmp"), PATH_STYLE_NATIVE));
#endif
}

typedef struct path_xdg_dir_info
{
    const char *env, *def;
} path_xdg_dir_info_st;

static const path_xdg_dir_info_st path_xdg_dir_base_infos[] =
{
      [PATH_BASE_DIR_CACHE]     = { "XDG_CACHE_HOME",       ".cache" }
    , [PATH_BASE_DIR_CONFIG]    = { "XDG_CONFIG_HOME",      ".config" }
    , [PATH_BASE_DIR_DATA]      = { "XDG_DATA_HOME",        ".local/share" }
    , [PATH_BASE_DIR_RUNTIME]   = { "XDG_RUNTIME_DIR",      NULL }
};

static const path_xdg_dir_info_st path_xdg_dir_user_infos[] =
{
      [PATH_USER_DIR_DESKTOP]   = { "XDG_DESKTOP_DIR",      "Desktop" }
    , [PATH_USER_DIR_DOCUMENTS] = { "XDG_DOCUMENTS_DIR",    "Documents" }
    , [PATH_USER_DIR_DOWNLOADS] = { "XDG_DOWNLOAD_DIR",     "Downloads" }
    , [PATH_USER_DIR_MUSIC]     = { "XDG_MUSIC_DIR",        "Music" }
    , [PATH_USER_DIR_PICTURES]  = { "XDG_PICTURES_DIR",     "Pictures" }
    , [PATH_USER_DIR_PUBLIC]    = { "XDG_PUBLICSHARE_DIR",  "Public" }
    , [PATH_USER_DIR_TEMPLATES] = { "XDG_TEMPLATES_DIR",    "Templates" }
    , [PATH_USER_DIR_VIDEOS]    = { "XDG_VIDEOS_DIR",       "Videos" }
};

static path_ct path_get_xdg_dir(const path_xdg_dir_info_st *info, bool def)
{
    str_const_ct value;
    path_ct path;
    
    if((value = env_get(STR(info->env))))
        return error_wrap_ptr(path_new(value, PATH_STYLE_NATIVE));
    
    if(error_code(0) != E_ENV_NOT_FOUND)
        return error_wrap(), NULL;
    
    if(!def || !info->def)
        return error_set(E_PATH_NOT_AVAILABLE), NULL;
    
    if(!(path = path_get_home()))
        return error_pass(), NULL;
    
    if(!path_append_c(path, info->def, PATH_STYLE_POSIX))
        return error_wrap(), path_free(path), NULL;
    
    return path;
}

#ifdef _WIN32

typedef struct path_win_dir_info
{
    const char *env;
    const KNOWNFOLDERID *id;
} path_win_dir_info_st;

static const path_win_dir_info_st path_win_dir_base_infos[] =
{
      [PATH_BASE_DIR_CACHE]     = { "LOCALAPPDATA", &FOLDERID_LocalAppData }
    , [PATH_BASE_DIR_CONFIG]    = { "APPDATA",      &FOLDERID_RoamingAppData }
    , [PATH_BASE_DIR_DATA]      = { "APPDATA",      &FOLDERID_RoamingAppData }
    , [PATH_BASE_DIR_RUNTIME]   = { NULL,           NULL }
};

static const path_win_dir_info_st path_win_dir_user_infos[] =
{
      [PATH_USER_DIR_DESKTOP]   = { NULL, &FOLDERID_Desktop }
    , [PATH_USER_DIR_DOCUMENTS] = { NULL, &FOLDERID_Documents }
    , [PATH_USER_DIR_DOWNLOADS] = { NULL, &FOLDERID_Downloads }
    , [PATH_USER_DIR_MUSIC]     = { NULL, &FOLDERID_Music }
    , [PATH_USER_DIR_PICTURES]  = { NULL, &FOLDERID_Pictures }
    , [PATH_USER_DIR_PUBLIC]    = { NULL, &FOLDERID_Public }
    , [PATH_USER_DIR_TEMPLATES] = { NULL, &FOLDERID_Templates }
    , [PATH_USER_DIR_VIDEOS]    = { NULL, &FOLDERID_Videos }
};

static path_ct path_get_win_dir(const path_win_dir_info_st *info, bool def)
{
    str_const_ct value;
    
    if(info->env && (value = env_get(STR(info->env))))
        return error_wrap_ptr(path_new(value, PATH_STYLE_NATIVE));
    
    if(info->env && error_code(0) != E_ENV_NOT_FOUND)
        return error_wrap(), NULL;
    
    if(!def || !info->id)
        return error_set(E_PATH_NOT_AVAILABLE), NULL;
    
    return error_pass_ptr(path_get_win_folder(info->id));
}

static path_ct path_get_xdg_win_dir(const path_xdg_dir_info_st *xdg, const path_win_dir_info_st *win)
{
    path_ct path;
    
    // use XDG directories/defaults if HOME available (MSYS)
    if(env_get(LIT("HOME")))
        return error_pass_ptr(path_get_xdg_dir(xdg, true));
    
    if(error_code(0) != E_ENV_NOT_FOUND)
        return error_wrap(), NULL;
    
    // recognize XDG directories if available
    if((path = path_get_xdg_dir(xdg, false)))
        return path;
    
    if(error_code(0) != E_PATH_NOT_AVAILABLE)
        return error_wrap(), NULL;
    
    // use standard windows directories
    return error_pass_ptr(path_get_win_dir(win, true));
}

#endif // _WIN32

path_ct path_get_base_dir(path_base_dir_id id)
{
    assert(id < PATH_BASE_DIRS);
    
    switch(id)
    {
    case PATH_BASE_DIR_HOME:
        return error_pass_ptr(path_get_home());
    case PATH_BASE_DIR_TMP:
        return error_pass_ptr(path_get_tmp());
    default:
#ifndef _WIN32
        return error_pass_ptr(path_get_xdg_dir(&path_xdg_dir_base_infos[id], true));
#else
        return error_pass_ptr(path_get_xdg_win_dir(&path_xdg_dir_base_infos[id], &path_win_dir_base_infos[id]));
#endif
    }
}

path_ct path_get_user_dir(path_user_dir_id id)
{
    assert(id < PATH_USER_DIRS);
    
#ifndef _WIN32
    return error_pass_ptr(path_get_xdg_dir(&path_xdg_dir_user_infos[id], true));
#else
    return error_pass_ptr(path_get_xdg_win_dir(&path_xdg_dir_user_infos[id], &path_win_dir_user_infos[id]));
#endif
}

typedef struct path_app_dir_info
{
    path_base_dir_id base;
    const char *xdg, *win;
} path_app_dir_info_st;

static const path_app_dir_info_st path_app_dir_infos[] =
{
      [PATH_APP_DIR_CACHE]   = { PATH_BASE_DIR_CACHE,   NULL,   "cache" }
    , [PATH_APP_DIR_CONFIG]  = { PATH_BASE_DIR_CONFIG,  NULL,   "config" }
    , [PATH_APP_DIR_DATA]    = { PATH_BASE_DIR_DATA,    NULL,   "data" }
    , [PATH_APP_DIR_LOG]     = { PATH_BASE_DIR_CACHE,   "logs", "logs" }
    , [PATH_APP_DIR_RUNTIME] = { PATH_BASE_DIR_RUNTIME, NULL,   NULL }
    , [PATH_APP_DIR_TMP]     = { PATH_BASE_DIR_TMP,     NULL,   NULL }
};

path_ct path_get_app_dir(path_app_dir_id id, str_const_ct author, str_const_ct app, str_const_ct version)
{
    const path_app_dir_info_st *info;
    path_ct path;
    const char *sub;
    
    assert(id < PATH_APP_DIRS);
    return_error_if_pass(author && str_is_empty(author), E_PATH_INVALID_APP_AUTHOR, NULL);
    return_error_if_pass(app && str_is_empty(app), E_PATH_INVALID_APP_NAME, NULL);
    return_error_if_pass(version && str_is_empty(version), E_PATH_INVALID_APP_VERSION, NULL);
    
    info = &path_app_dir_infos[id];
    
    if(!(path = path_get_base_dir(info->base)))
        return error_pass(), NULL;
    
#ifdef _WIN32
    sub = env_get(LIT("HOME")) ? info->xdg : info->win;
#else
    sub = info->xdg;
#endif
    
    if((author && !path_append(path, author, PATH_STYLE_NATIVE))
    || (app && !path_append(path, app, PATH_STYLE_NATIVE))
    || (version && !path_append(path, version, PATH_STYLE_NATIVE))
    || (sub && !path_append_c(path, sub, PATH_STYLE_POSIX)))
        return error_wrap(), path_free(path), NULL;
    
    return path;
}
