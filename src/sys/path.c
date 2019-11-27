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
#   include <initguid.h>
#   include <KnownFolders.h>
#   include <ShlObj.h>
#else
#   include <unistd.h>
#   include <pwd.h>
#endif

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_PATH_INVALID_APP_AUTHOR, "Invalid app author.")
    , ERROR_INFO(E_PATH_INVALID_APP_NAME, "Invalid app name.")
    , ERROR_INFO(E_PATH_INVALID_APP_VERSION, "Invalid app version.")
    , ERROR_INFO(E_PATH_NOT_AVAILABLE, "Path not available.")
};


#ifdef _WIN32
static path_ct path_get_windows_folder(const KNOWNFOLDERID *id)
{
    wchar_t *wfolder = NULL;
    char *cfolder;
    ssize_t len;
    HRESULT rc;
    
    if((rc = SHGetKnownFolderPath(id, KF_FLAG_DEFAULT, NULL, &wfolder)) != S_OK)
        return error_wrap_hresult(SHGetKnownFolderPath, rc), NULL;
    
    if((len = wcstombs(NULL, wfolder, 0)) < 0)
        return errno = EILSEQ, error_wrap_errno(wcstombs), CoTaskMemFree(wfolder), NULL;
    
    cfolder = alloca(len+1);
    wcstombs(cfolder, wfolder, len+1);
    CoTaskMemFree(wfolder);
    
    return error_wrap_ptr(path_new_c(cfolder, PATH_STYLE_NATIVE));
}
#endif

path_ct path_get_user_home(void)
{
    str_const_ct value;
    
    if((value = env_get(LIT("HOME"))))
        return error_wrap_ptr(path_new(value, PATH_STYLE_NATIVE));
    
#ifdef _WIN32
    return error_pass_ptr(path_get_windows_folder(&FOLDERID_Profile));
#else
    struct passwd *pwd;
    
    if(!(pwd = getpwuid(getuid())))
        return error_wrap_errno(getpwuid), NULL;
    
    return error_wrap_ptr(path_new_c(pwd->pw_dir, PATH_STYLE_NATIVE));
#endif
}

typedef struct path_user_dir_xdg_info
{
    const char *env, *def;
} path_user_dir_xdg_info_st;

static const path_user_dir_xdg_info_st path_user_dir_xdg_infos[] =
{
      [PATH_USER_DIR_DESKTOP]   = { "XDG_DESKTOP_DIR",     "Desktop" }
    , [PATH_USER_DIR_DOCUMENTS] = { "XDG_DOCUMENTS_DIR",   "Documents" }
    , [PATH_USER_DIR_DOWNLOADS] = { "XDG_DOWNLOAD_DIR",    "Downloads" }
    , [PATH_USER_DIR_MUSIC]     = { "XDG_MUSIC_DIR",       "Music" }
    , [PATH_USER_DIR_PICTURES]  = { "XDG_PICTURES_DIR",    "Pictures" }
    , [PATH_USER_DIR_PUBLIC]    = { "XDG_PUBLICSHARE_DIR", "Public" }
    , [PATH_USER_DIR_TEMPLATES] = { "XDG_TEMPLATES_DIR",   "Templates" }
    , [PATH_USER_DIR_VIDEOS]    = { "XDG_VIDEOS_DIR",      "Videos" }
};

static path_ct path_get_user_dir_xdg(path_user_dir_id id, bool def)
{
    const path_user_dir_xdg_info_st *info = &path_user_dir_xdg_infos[id];
    str_const_ct value;
    path_ct path;
    
    if((value = env_get(STR(info->env))))
        return error_wrap_ptr(path_new(value, PATH_STYLE_NATIVE));
    
    if(!def)
        return error_set(E_PATH_NOT_AVAILABLE), NULL;
    
    if(!(path = path_get_user_home()))
        return error_pass(), NULL;
    
    if(!path_append_c(path, info->def, PATH_STYLE_POSIX))
        return error_wrap(), path_free(path), NULL;
    
    return path;
}

#ifdef _WIN32

typedef struct path_user_dir_windows_info
{
    const KNOWNFOLDERID *id;
} path_user_dir_windows_info_st;

static const path_user_dir_windows_info_st path_user_dir_windows_infos[] =
{
      [PATH_USER_DIR_DESKTOP]   = { &FOLDERID_Desktop }
    , [PATH_USER_DIR_DOCUMENTS] = { &FOLDERID_Documents }
    , [PATH_USER_DIR_DOWNLOADS] = { &FOLDERID_Downloads }
    , [PATH_USER_DIR_MUSIC]     = { &FOLDERID_Music }
    , [PATH_USER_DIR_PICTURES]  = { &FOLDERID_Pictures }
    , [PATH_USER_DIR_PUBLIC]    = { &FOLDERID_Public }
    , [PATH_USER_DIR_TEMPLATES] = { &FOLDERID_Templates }
    , [PATH_USER_DIR_VIDEOS]    = { &FOLDERID_Videos }
};

static path_ct path_get_user_dir_windows(path_user_dir_id id)
{
    const path_user_dir_windows_info_st *info = &path_user_dir_windows_infos[id];
    
    return error_pass_ptr(path_get_windows_folder(info->id));
}

#endif // _WIN32

path_ct path_get_user_dir(path_user_dir_id id)
{
    assert(id < PATH_USER_DIRS);
    
#ifndef _WIN32
    return error_pass_ptr(path_get_user_dir_xdg(id, true));
#else
    path_ct path;
    
    // use XDG directories/defaults if HOME available (MSYS)
    if(env_get(LIT("HOME")))
        return error_pass_ptr(path_get_user_dir_xdg(id, true));
    
    if(!error_check(0, E_ENV_NOT_FOUND))
        return error_pass(), NULL;
    
    // recognize XDG directories if available
    if((path = path_get_user_dir_xdg(id, false)))
        return path;
    
    if(!error_check(0, E_PATH_NOT_AVAILABLE))
        return error_pass(), NULL;
    
    // use standard windows directories
    return error_pass_ptr(path_get_user_dir_windows(id));
#endif
}

typedef struct path_app_dir_xdg_info
{
    const char *env, *sub, *def, *sub_def;
} path_app_dir_xdg_info_st;

static const path_app_dir_xdg_info_st path_app_dir_xdg_infos[] =
{
      [PATH_APP_DIR_CACHE]   = { "XDG_CACHE_HOME",  NULL,   ".cache",       NULL }
    , [PATH_APP_DIR_CONFIG]  = { "XDG_CONFIG_HOME", NULL,   ".config",      NULL }
    , [PATH_APP_DIR_DATA]    = { "XDG_DATA_HOME",   NULL,   ".local/share", NULL }
    , [PATH_APP_DIR_LOG]     = { "XDG_CACHE_HOME",  "logs", ".cache",       "logs" }
    , [PATH_APP_DIR_RUNTIME] = { "XDG_RUNTIME_DIR", NULL,   ".cache",       "run" }
};

static path_ct path_get_app_dir_xdg(path_app_dir_id id, str_const_ct app, str_const_ct version, bool def)
{
    const path_app_dir_xdg_info_st *info = &path_app_dir_xdg_infos[id];
    str_const_ct value;
    path_ct path;
    
    if((value = env_get(STR(info->env))))
    {
        if(!(path = path_new(value, PATH_STYLE_NATIVE)))
            return error_wrap(), NULL;
        
        if(!path_append(path, app, PATH_STYLE_NATIVE)
        || (version && !path_append(path, version, PATH_STYLE_NATIVE))
        || (info->sub && !path_append_c(path, info->sub, PATH_STYLE_POSIX)))
            return error_wrap(), path_free(path), NULL;
    }
    else if(def)
    {
        if(!(path = path_get_user_home()))
            return error_pass(), NULL;
        
        if(!path_append_c(path, info->def, PATH_STYLE_POSIX)
        || !path_append(path, app, PATH_STYLE_NATIVE)
        || (version && !path_append(path, version, PATH_STYLE_NATIVE))
        || (info->sub_def && !path_append_c(path, info->sub_def, PATH_STYLE_POSIX)))
            return error_wrap(), path_free(path), NULL;
    }
    else
        return error_set(E_PATH_NOT_AVAILABLE), NULL;
    
    return path;
}

#ifdef _WIN32

typedef struct path_app_dir_windows_info
{
    const char *env;
    const KNOWNFOLDERID *id;
    const char *sub;
} path_app_dir_windows_info_st;

static const path_app_dir_windows_info_st path_app_dir_windows_infos[] =
{
      [PATH_APP_DIR_CACHE]   = { "LOCALAPPDATA", &FOLDERID_LocalAppData,   "cache" }
    , [PATH_APP_DIR_CONFIG]  = { "APPDATA",      &FOLDERID_RoamingAppData, NULL }
    , [PATH_APP_DIR_DATA]    = { "LOCALAPPDATA", &FOLDERID_LocalAppData,   NULL }
    , [PATH_APP_DIR_LOG]     = { "LOCALAPPDATA", &FOLDERID_LocalAppData,   "logs" }
    , [PATH_APP_DIR_RUNTIME] = { "LOCALAPPDATA", &FOLDERID_LocalAppData,   "run" }
};

static path_ct path_get_app_dir_windows(path_app_dir_id id, str_const_ct author, str_const_ct app, str_const_ct version)
{
    const path_app_dir_windows_info_st *info = &path_app_dir_windows_infos[id];
    str_const_ct value;
    path_ct path;
    
    if((value = env_get(STR(info->env))))
    {
        if(!(path = path_new(value, PATH_STYLE_NATIVE)))
            return error_wrap(), NULL;
    }
    else if(!(path = path_get_windows_folder(info->id)))
        return error_pass(), NULL;
    
    if(!path_append(path, author, PATH_STYLE_NATIVE)
    || !path_append(path, app, PATH_STYLE_NATIVE)
    || (version && !path_append(path, version, PATH_STYLE_NATIVE))
    || (info->sub && !path_append_c(path, info->sub, PATH_STYLE_POSIX)))
        return error_wrap(), path_free(path), NULL;
    
    return path;
}

#endif // _WIN32

path_ct path_get_app_dir(path_app_dir_id id, str_const_ct author, str_const_ct app, str_const_ct version)
{
    assert(id < PATH_APP_DIRS);
    return_error_if_pass(str_is_empty(author), E_PATH_INVALID_APP_AUTHOR, NULL);
    return_error_if_pass(str_is_empty(app), E_PATH_INVALID_APP_NAME, NULL);
    return_error_if_pass(version && str_is_empty(version), E_PATH_INVALID_APP_VERSION, NULL);
    
#ifndef _WIN32
    return error_pass_ptr(path_get_app_dir_xdg(id, app, version, true));
#else
    path_ct path;
    
    // use XDG directories/defaults if HOME available (MSYS)
    if(env_get(LIT("HOME")))
        return error_pass_ptr(path_get_app_dir_xdg(id, app, version, true));
    
    if(!error_check(0, E_ENV_NOT_FOUND))
        return error_pass(), NULL;
    
    // recognize XDG directories if available
    if((path = path_get_app_dir_xdg(id, app, version, false)))
        return path;
    
    if(!error_check(0, E_PATH_NOT_AVAILABLE))
        return error_pass(), NULL;
    
    // use standard windows directories
    return error_pass_ptr(path_get_app_dir_windows(id, author, app, version));
#endif
}
