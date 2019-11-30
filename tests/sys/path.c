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

#include "path.h"
#include <ytil/test/test.h>
#include <ytil/sys/path.h>
#include <ytil/sys/env.h>

#ifdef _WIN32
#   include <initguid.h>
#   include <KnownFolders.h>
#   include <ShlObj.h>
#else
#   include <unistd.h>
#   include <pwd.h>
#   include <stdio.h>
#   include <limits.h>
#   define MAX_PATH PATH_MAX
#endif

static char buf[MAX_PATH];
static const char *cstr;
static path_ct path;
static str_const_ct cpath;


#ifdef _WIN32
static char *_test_path_get_windows_folder(const KNOWNFOLDERID *id, const char *append)
{
    wchar_t *wfolder = NULL;
    
    SHGetKnownFolderPath(id, KF_FLAG_DEFAULT, NULL, &wfolder);
    wcstombs(buf, wfolder, MAX_PATH);
    CoTaskMemFree(wfolder);
    strcat(buf, append);
    
    return buf;
}
#endif

TEST_SETUP(env_init)
{
    env_init();
}

TEST_TEARDOWN(env_free)
{
    env_free();
}

TEST_CASE_ABORT(path_get_base_dir_invalid_ident)
{
    path_get_base_dir(999);
}

TEST_CASE_FIXTURE(path_get_base_dir_home_home_set_profile_set, env_init, env_free)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_int_success(env_set(LIT("USERPROFILE"), LIT("/users/bar")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_HOME));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE_FIXTURE(path_get_base_dir_home_home_unset_profile_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("USERPROFILE"), LIT("/users/bar")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_HOME));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/bar");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE(path_get_base_dir_home_home_unset_profile_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("USERPROFILE")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_HOME));
    
#ifdef _WIN32
    cstr = _test_path_get_windows_folder(&FOLDERID_Profile, "");
#else
    cstr = getpwuid(getuid())->pw_dir;
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), cstr);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_base_dir_tmp_tmp_set_temp_set, env_init, env_free)
{
    test_int_success(env_set(LIT("TMP"), LIT("/foo/tmp")));
    test_int_success(env_set(LIT("TEMP"), LIT("/bar/tmp")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/foo/tmp");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_base_dir_tmp_tmp_unset_temp_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("TMP")));
    test_int_success(env_set(LIT("TEMP"), LIT("/bar/tmp")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/bar/tmp");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_base_dir_tmp_tmp_unset_temp_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("TMP")));
    test_int_success(env_unset(LIT("TEMP")));
    test_ptr_error(path_get_base_dir(PATH_BASE_DIR_TMP), E_PATH_NOT_AVAILABLE);
}

TEST_CASE_FIXTURE(path_get_base_dir_cache_xdg_set_home_set_win_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_CACHE_HOME"), LIT("/home/foo/my_cache")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_CACHE));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_cache");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_base_dir_cache_xdg_unset_home_set_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_CACHE));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.cache");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE_FIXTURE(path_get_base_dir_cache_xdg_unset_home_unset_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_CACHE));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/local");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE(path_get_base_dir_cache_xdg_unset_home_unset_win_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_CACHE));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_LocalAppData, "");
#else
    sprintf(buf, "%s/.cache", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_base_dir_config_xdg_set_home_set_win_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_CONFIG_HOME"), LIT("/home/foo/my_config")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_CONFIG));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_config");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_base_dir_config_xdg_unset_home_set_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_CONFIG));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.config");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE_FIXTURE(path_get_base_dir_config_xdg_unset_home_unset_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_CONFIG));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/roaming");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE(path_get_base_dir_config_xdg_unset_home_unset_win_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("APPDATA")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_CONFIG));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_RoamingAppData, "");
#else
    sprintf(buf, "%s/.config", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_base_dir_data_xdg_set_home_set_win_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_DATA_HOME"), LIT("/home/foo/my_data")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_DATA));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_data");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_base_dir_data_xdg_unset_home_set_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_DATA));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.local/share");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE_FIXTURE(path_get_base_dir_data_xdg_unset_home_unset_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_DATA));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/roaming");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE(path_get_base_dir_data_xdg_unset_home_unset_win_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("APPDATA")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_DATA));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_RoamingAppData, "");
#else
    sprintf(buf, "%s/.local/share", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_base_dir_runtime_xdg_set_home_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_RUNTIME_DIR"), LIT("/home/foo/my_run")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_RUNTIME));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_run");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_base_dir_runtime_xdg_unset_home_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_RUNTIME_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_error(path_get_base_dir(PATH_BASE_DIR_RUNTIME), E_PATH_NOT_AVAILABLE);
}

TEST_CASE_FIXTURE(path_get_base_dir_runtime_xdg_unset_home_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_RUNTIME_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_error(path_get_base_dir(PATH_BASE_DIR_RUNTIME), E_PATH_NOT_AVAILABLE);
}

TEST_CASE_ABORT(path_get_user_dir_invalid_ident)
{
    path_get_user_dir(999);
}

TEST_CASE_FIXTURE(path_get_user_dir_desktop_xdg_set_home_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_DESKTOP_DIR"), LIT("/home/foo/my_desktop")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DESKTOP));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_desktop");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_desktop_xdg_unset_home_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DESKTOP_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DESKTOP));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Desktop");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_desktop_xdg_unset_home_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DESKTOP_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DESKTOP));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_Desktop, "");
#else
    sprintf(buf, "%s/Desktop", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_documents_xdg_set_home_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_DOCUMENTS_DIR"), LIT("/home/foo/my_documents")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOCUMENTS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_documents");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_documents_xdg_unset_home_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DOCUMENTS_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOCUMENTS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Documents");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_documents_xdg_unset_home_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DOCUMENTS_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOCUMENTS));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_Documents, "");
#else
    sprintf(buf, "%s/Documents", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_downloads_xdg_set_home_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_DOWNLOAD_DIR"), LIT("/home/foo/my_downloads")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOWNLOADS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_downloads");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_downloads_xdg_unset_home_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DOWNLOAD_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOWNLOADS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Downloads");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_downloads_xdg_unset_home_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DOWNLOAD_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOWNLOADS));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_Downloads, "");
#else
    sprintf(buf, "%s/Downloads", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_music_xdg_set_home_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_MUSIC_DIR"), LIT("/home/foo/my_music")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_MUSIC));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_music");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_music_xdg_unset_home_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_MUSIC_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_MUSIC));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Music");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_music_xdg_unset_home_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_MUSIC_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_MUSIC));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_Music, "");
#else
    sprintf(buf, "%s/Music", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_pictures_xdg_set_home_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_PICTURES_DIR"), LIT("/home/foo/my_pictures")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PICTURES));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_pictures");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_pictures_xdg_unset_home_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_PICTURES_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PICTURES));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Pictures");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_pictures_xdg_unset_home_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_PICTURES_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PICTURES));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_Pictures, "");
#else
    sprintf(buf, "%s/Pictures", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_public_xdg_set_home_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_PUBLICSHARE_DIR"), LIT("/home/foo/my_share")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PUBLIC));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_share");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_public_xdg_unset_home_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_PUBLICSHARE_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PUBLIC));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Public");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_public_xdg_unset_home_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_PUBLICSHARE_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PUBLIC));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_Public, "");
#else
    sprintf(buf, "%s/Public", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_templates_xdg_set_home_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_TEMPLATES_DIR"), LIT("/home/foo/my_templates")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_TEMPLATES));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_templates");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_templates_xdg_unset_home_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_TEMPLATES_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_TEMPLATES));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Templates");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_templates_xdg_unset_home_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_TEMPLATES_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_TEMPLATES));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_Templates, "");
#else
    sprintf(buf, "%s/Templates", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_videos_xdg_set_home_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_VIDEOS_DIR"), LIT("/home/foo/my_videos")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_VIDEOS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_videos");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_videos_xdg_unset_home_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_VIDEOS_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_VIDEOS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Videos");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_user_dir_videos_xdg_unset_home_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_VIDEOS_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_VIDEOS));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_Videos, "");
#else
    sprintf(buf, "%s/Videos", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_ABORT(path_get_app_dir_invalid_ident)
{
    path_get_app_dir(999, LIT("ACME"), LIT("tron"), LIT("1.2.3"));
}

TEST_CASE(path_get_app_dir_invalid_author)
{
    test_ptr_error(path_get_app_dir(PATH_APP_DIR_CACHE, LIT(""), LIT("tron"), LIT("1.2.3")), E_PATH_INVALID_APP_AUTHOR);
}

TEST_CASE(path_get_app_dir_invalid_name)
{
    test_ptr_error(path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT(""), LIT("1.2.3")), E_PATH_INVALID_APP_NAME);
}

TEST_CASE(path_get_app_dir_invalid_version)
{
    test_ptr_error(path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), LIT("")), E_PATH_INVALID_APP_VERSION);
}

TEST_CASE_FIXTURE(path_get_app_dir_cache_xdg_set_home_set_win_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_CACHE_HOME"), LIT("/home/foo/my_cache")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_cache/ACME/tron/1.2.3");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_cache_xdg_unset_home_set_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.cache/ACME/tron/1.2.3");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE_FIXTURE(path_get_app_dir_cache_xdg_unset_home_unset_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/local/ACME/tron/1.2.3/cache");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE(path_get_app_dir_cache_xdg_unset_home_unset_win_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_LocalAppData, "\\ACME\\tron\\1.2.3\\cache");
#else
    sprintf(buf, "%s/.cache/ACME/tron/1.2.3", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_config_xdg_set_home_set_win_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_CONFIG_HOME"), LIT("/home/foo/my_config")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_config/ACME/tron/1.2.3");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_config_xdg_unset_home_set_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.config/ACME/tron/1.2.3");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE_FIXTURE(path_get_app_dir_config_xdg_unset_home_unset_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/roaming/ACME/tron/1.2.3/config");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE(path_get_app_dir_config_xdg_unset_home_unset_win_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("APPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_RoamingAppData, "\\ACME\\tron\\1.2.3\\config");
#else
    sprintf(buf, "%s/.config/ACME/tron/1.2.3", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_data_xdg_set_home_set_win_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_DATA_HOME"), LIT("/home/foo/my_data")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_data/ACME/tron/1.2.3");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_data_xdg_unset_home_set_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.local/share/ACME/tron/1.2.3");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE_FIXTURE(path_get_app_dir_data_xdg_unset_home_unset_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/roaming/ACME/tron/1.2.3/data");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE(path_get_app_dir_data_xdg_unset_home_unset_win_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("APPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_RoamingAppData, "\\ACME\\tron\\1.2.3\\data");
#else
    sprintf(buf, "%s/.local/share/ACME/tron/1.2.3", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_log_xdg_set_home_set_win_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_CACHE_HOME"), LIT("/home/foo/my_cache")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_cache/ACME/tron/1.2.3/logs");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_log_xdg_unset_home_set_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.cache/ACME/tron/1.2.3/logs");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE_FIXTURE(path_get_app_dir_log_xdg_unset_home_unset_win_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/local/ACME/tron/1.2.3/logs");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE_FIXTURE(path_get_app_dir_log_xdg_unset_home_unset_win_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    
#ifdef _WIN32
    _test_path_get_windows_folder(&FOLDERID_LocalAppData, "\\ACME\\tron\\1.2.3\\logs");
#else
    sprintf(buf, "%s/.cache/ACME/tron/1.2.3/logs", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_runtime_xdg_set_home_set, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_RUNTIME_DIR"), LIT("/home/foo/my_run")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_RUNTIME, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_run/ACME/tron/1.2.3");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_runtime_xdg_unset_home_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_RUNTIME_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_ptr_error(path_get_app_dir(PATH_APP_DIR_RUNTIME, LIT("ACME"), LIT("tron"), LIT("1.2.3")), E_PATH_NOT_AVAILABLE);
}

TEST_CASE_FIXTURE(path_get_app_dir_runtime_xdg_unset_home_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_RUNTIME_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_error(path_get_app_dir(PATH_APP_DIR_RUNTIME, LIT("ACME"), LIT("tron"), LIT("1.2.3")), E_PATH_NOT_AVAILABLE);
}

TEST_CASE_FIXTURE(path_get_app_dir_tmp_tmp_set_temp_set, env_init, env_free)
{
    test_int_success(env_set(LIT("TMP"), LIT("/foo/tmp")));
    test_int_success(env_set(LIT("TEMP"), LIT("/bar/tmp")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_TMP, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/foo/tmp/ACME/tron/1.2.3");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_tmp_tmp_unset_temp_set, env_init, env_free)
{
    test_int_success(env_unset(LIT("TMP")));
    test_int_success(env_set(LIT("TEMP"), LIT("/bar/tmp")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_TMP, LIT("ACME"), LIT("tron"), LIT("1.2.3")));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/bar/tmp/ACME/tron/1.2.3");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_FIXTURE(path_get_app_dir_tmp_tmp_unset_temp_unset, env_init, env_free)
{
    test_int_success(env_unset(LIT("TMP")));
    test_int_success(env_unset(LIT("TEMP")));
    test_ptr_error(path_get_app_dir(PATH_APP_DIR_TMP, LIT("ACME"), LIT("tron"), LIT("1.2.3")), E_PATH_NOT_AVAILABLE);
}

test_suite_ct test_suite_sys_path(void)
{
    return test_suite_new_with_cases("path"
        , test_case_new(path_get_base_dir_invalid_ident)
        
        , test_case_new(path_get_base_dir_home_home_set_profile_set)
        , test_case_new_windows(path_get_base_dir_home_home_unset_profile_set)
        , test_case_new(path_get_base_dir_home_home_unset_profile_unset)
        
        , test_case_new(path_get_base_dir_tmp_tmp_set_temp_set)
        , test_case_new(path_get_base_dir_tmp_tmp_unset_temp_set)
        , test_case_new(path_get_base_dir_tmp_tmp_unset_temp_unset)
        
        , test_case_new(path_get_base_dir_cache_xdg_set_home_set_win_set)
        , test_case_new(path_get_base_dir_cache_xdg_unset_home_set_win_set)
        , test_case_new_windows(path_get_base_dir_cache_xdg_unset_home_unset_win_set)
        , test_case_new(path_get_base_dir_cache_xdg_unset_home_unset_win_unset)
        
        , test_case_new(path_get_base_dir_config_xdg_set_home_set_win_set)
        , test_case_new(path_get_base_dir_config_xdg_unset_home_set_win_set)
        , test_case_new_windows(path_get_base_dir_config_xdg_unset_home_unset_win_set)
        , test_case_new(path_get_base_dir_config_xdg_unset_home_unset_win_unset)
        
        , test_case_new(path_get_base_dir_data_xdg_set_home_set_win_set)
        , test_case_new(path_get_base_dir_data_xdg_unset_home_set_win_set)
        , test_case_new_windows(path_get_base_dir_data_xdg_unset_home_unset_win_set)
        , test_case_new(path_get_base_dir_data_xdg_unset_home_unset_win_unset)
        
        , test_case_new(path_get_base_dir_runtime_xdg_set_home_set)
        , test_case_new(path_get_base_dir_runtime_xdg_unset_home_set)
        , test_case_new(path_get_base_dir_runtime_xdg_unset_home_unset)
        
        
        , test_case_new(path_get_user_dir_invalid_ident)
        
        , test_case_new(path_get_user_dir_desktop_xdg_set_home_set)
        , test_case_new(path_get_user_dir_desktop_xdg_unset_home_set)
        , test_case_new(path_get_user_dir_desktop_xdg_unset_home_unset)
        
        , test_case_new(path_get_user_dir_documents_xdg_set_home_set)
        , test_case_new(path_get_user_dir_documents_xdg_unset_home_set)
        , test_case_new(path_get_user_dir_documents_xdg_unset_home_unset)
        
        , test_case_new(path_get_user_dir_downloads_xdg_set_home_set)
        , test_case_new(path_get_user_dir_downloads_xdg_unset_home_set)
        , test_case_new(path_get_user_dir_downloads_xdg_unset_home_unset)
        
        , test_case_new(path_get_user_dir_music_xdg_set_home_set)
        , test_case_new(path_get_user_dir_music_xdg_unset_home_set)
        , test_case_new(path_get_user_dir_music_xdg_unset_home_unset)
        
        , test_case_new(path_get_user_dir_pictures_xdg_set_home_set)
        , test_case_new(path_get_user_dir_pictures_xdg_unset_home_set)
        , test_case_new(path_get_user_dir_pictures_xdg_unset_home_unset)
        
        , test_case_new(path_get_user_dir_public_xdg_set_home_set)
        , test_case_new(path_get_user_dir_public_xdg_unset_home_set)
        , test_case_new(path_get_user_dir_public_xdg_unset_home_unset)
        
        , test_case_new(path_get_user_dir_templates_xdg_set_home_set)
        , test_case_new(path_get_user_dir_templates_xdg_unset_home_set)
        , test_case_new(path_get_user_dir_templates_xdg_unset_home_unset)
        
        , test_case_new(path_get_user_dir_videos_xdg_set_home_set)
        , test_case_new(path_get_user_dir_videos_xdg_unset_home_set)
        , test_case_new(path_get_user_dir_videos_xdg_unset_home_unset)
        
        
        , test_case_new(path_get_app_dir_invalid_ident)
        , test_case_new(path_get_app_dir_invalid_author)
        , test_case_new(path_get_app_dir_invalid_name)
        , test_case_new(path_get_app_dir_invalid_version)
        
        , test_case_new(path_get_app_dir_cache_xdg_set_home_set_win_set)
        , test_case_new(path_get_app_dir_cache_xdg_unset_home_set_win_set)
        , test_case_new_windows(path_get_app_dir_cache_xdg_unset_home_unset_win_set)
        , test_case_new(path_get_app_dir_cache_xdg_unset_home_unset_win_unset)
        
        , test_case_new(path_get_app_dir_config_xdg_set_home_set_win_set)
        , test_case_new(path_get_app_dir_config_xdg_unset_home_set_win_set)
        , test_case_new_windows(path_get_app_dir_config_xdg_unset_home_unset_win_set)
        , test_case_new(path_get_app_dir_config_xdg_unset_home_unset_win_unset)
        
        , test_case_new(path_get_app_dir_data_xdg_set_home_set_win_set)
        , test_case_new(path_get_app_dir_data_xdg_unset_home_set_win_set)
        , test_case_new_windows(path_get_app_dir_data_xdg_unset_home_unset_win_set)
        , test_case_new(path_get_app_dir_data_xdg_unset_home_unset_win_unset)
        
        , test_case_new(path_get_app_dir_log_xdg_set_home_set_win_set)
        , test_case_new(path_get_app_dir_log_xdg_unset_home_set_win_set)
        , test_case_new_windows(path_get_app_dir_log_xdg_unset_home_unset_win_set)
        , test_case_new(path_get_app_dir_log_xdg_unset_home_unset_win_unset)
        
        , test_case_new(path_get_app_dir_runtime_xdg_set_home_set)
        , test_case_new(path_get_app_dir_runtime_xdg_unset_home_set)
        , test_case_new(path_get_app_dir_runtime_xdg_unset_home_unset)
        
        , test_case_new(path_get_app_dir_tmp_tmp_set_temp_set)
        , test_case_new(path_get_app_dir_tmp_tmp_unset_temp_set)
        , test_case_new(path_get_app_dir_tmp_tmp_unset_temp_unset)
    );
}
