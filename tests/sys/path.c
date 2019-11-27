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
#endif

char *buf;
const char *cstr;
path_ct path;
str_const_ct cpath;


#ifdef _WIN32
static char *_test_path_windows_folder(const KNOWNFOLDERID *id, char *buf)
{
    wchar_t *wfolder = NULL;
    
    SHGetKnownFolderPath(id, KF_FLAG_DEFAULT, NULL, &wfolder);
    wcstombs(buf, wfolder, MAX_PATH);
    CoTaskMemFree(wfolder);
    
    return buf;
}
#endif

TEST_CASE(path_get_user_home_env)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_ptr_success(path = path_get_user_home());
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_home_native)
{
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_home());
    
#ifdef _WIN32
    cstr = _test_path_windows_folder(&FOLDERID_Profile, alloca(MAX_PATH));
#else
    cstr = getpwuid(getuid())->pw_dir;
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), cstr);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_ABORT(path_get_user_dir_invalid_ident)
{
    path_get_user_dir(999);
}

TEST_CASE(path_get_user_dir_desktop_xdg_set_home_set)
{
    test_int_success(env_set(LIT("XDG_DESKTOP_DIR"), LIT("/home/foo/my_desktop")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DESKTOP));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_desktop");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_desktop_xdg_unset_home_set)
{
    test_int_success(env_unset(LIT("XDG_DESKTOP_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DESKTOP));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Desktop");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_desktop_xdg_unset_home_unset)
{
    test_int_success(env_unset(LIT("XDG_DESKTOP_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DESKTOP));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_Desktop, alloca(MAX_PATH));
#else
    sprintf(buf, "%s/Desktop", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_documents_xdg_set_home_set)
{
    test_int_success(env_set(LIT("XDG_DOCUMENTS_DIR"), LIT("/home/foo/my_documents")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOCUMENTS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_documents");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_documents_xdg_unset_home_set)
{
    test_int_success(env_unset(LIT("XDG_DOCUMENTS_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOCUMENTS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Documents");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_documents_xdg_unset_home_unset)
{
    test_int_success(env_unset(LIT("XDG_DOCUMENTS_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOCUMENTS));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_Documents, alloca(MAX_PATH));
#else
    sprintf(buf, "%s/Documents", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_downloads_xdg_set_home_set)
{
    test_int_success(env_set(LIT("XDG_DOWNLOAD_DIR"), LIT("/home/foo/my_downloads")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOWNLOADS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_downloads");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_downloads_xdg_unset_home_set)
{
    test_int_success(env_unset(LIT("XDG_DOWNLOAD_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOWNLOADS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Downloads");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_downloads_xdg_unset_home_unset)
{
    test_int_success(env_unset(LIT("XDG_DOWNLOAD_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_DOWNLOADS));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_Downloads, alloca(MAX_PATH));
#else
    sprintf(buf, "%s/Downloads", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_music_xdg_set_home_set)
{
    test_int_success(env_set(LIT("XDG_MUSIC_DIR"), LIT("/home/foo/my_music")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_MUSIC));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_music");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_music_xdg_unset_home_set)
{
    test_int_success(env_unset(LIT("XDG_MUSIC_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_MUSIC));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Music");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_music_xdg_unset_home_unset)
{
    test_int_success(env_unset(LIT("XDG_MUSIC_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_MUSIC));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_Music, alloca(MAX_PATH));
#else
    sprintf(buf, "%s/Music", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_pictures_xdg_set_home_set)
{
    test_int_success(env_set(LIT("XDG_PICTURES_DIR"), LIT("/home/foo/my_pictures")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PICTURES));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_pictures");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_pictures_xdg_unset_home_set)
{
    test_int_success(env_unset(LIT("XDG_PICTURES_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PICTURES));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Pictures");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_pictures_xdg_unset_home_unset)
{
    test_int_success(env_unset(LIT("XDG_PICTURES_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PICTURES));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_Pictures, alloca(MAX_PATH));
#else
    sprintf(buf, "%s/Pictures", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_public_xdg_set_home_set)
{
    test_int_success(env_set(LIT("XDG_PUBLICSHARE_DIR"), LIT("/home/foo/my_share")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PUBLIC));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_share");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_public_xdg_unset_home_set)
{
    test_int_success(env_unset(LIT("XDG_PUBLICSHARE_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PUBLIC));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Public");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_public_xdg_unset_home_unset)
{
    test_int_success(env_unset(LIT("XDG_PUBLICSHARE_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_PUBLIC));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_Public, alloca(MAX_PATH));
#else
    sprintf(buf, "%s/Public", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_templates_xdg_set_home_set)
{
    test_int_success(env_set(LIT("XDG_TEMPLATES_DIR"), LIT("/home/foo/my_templates")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_TEMPLATES));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_templates");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_templates_xdg_unset_home_set)
{
    test_int_success(env_unset(LIT("XDG_TEMPLATES_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_TEMPLATES));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Templates");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_templates_xdg_unset_home_unset)
{
    test_int_success(env_unset(LIT("XDG_TEMPLATES_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_TEMPLATES));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_Templates, alloca(MAX_PATH));
#else
    sprintf(buf, "%s/Templates", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_videos_xdg_set_home_set)
{
    test_int_success(env_set(LIT("XDG_VIDEOS_DIR"), LIT("/home/foo/my_videos")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_VIDEOS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_videos");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_videos_xdg_unset_home_set)
{
    test_int_success(env_unset(LIT("XDG_VIDEOS_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_VIDEOS));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/Videos");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_user_dir_videos_xdg_unset_home_unset)
{
    test_int_success(env_unset(LIT("XDG_VIDEOS_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_VIDEOS));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_Videos, alloca(MAX_PATH));
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
    path_get_app_dir(999, LIT("ACME"), LIT("tron"), NULL);
}

TEST_CASE_ABORT(path_get_app_dir_invalid_author1)
{
    path_get_app_dir(PATH_APP_DIR_CACHE, NULL, LIT("tron"), NULL);
}

TEST_CASE(path_get_app_dir_invalid_author2)
{
    test_ptr_error(path_get_app_dir(PATH_APP_DIR_CACHE, LIT(""), LIT("tron"), NULL), E_PATH_INVALID_APP_AUTHOR);
}

TEST_CASE_ABORT(path_get_app_dir_invalid_name1)
{
    path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), NULL, NULL);
}

TEST_CASE(path_get_app_dir_invalid_name2)
{
    test_ptr_error(path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT(""), NULL), E_PATH_INVALID_APP_NAME);
}

TEST_CASE(path_get_app_dir_invalid_version)
{
    test_ptr_error(path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), LIT("")), E_PATH_INVALID_APP_VERSION);
}

TEST_CASE(path_get_app_dir_cache_xdg_set_home_set_win_set)
{
    test_int_success(env_set(LIT("XDG_CACHE_HOME"), LIT("/home/foo/my_cache")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_cache/tron");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_cache_xdg_unset_home_set_win_set)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.cache/tron");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE(path_get_app_dir_cache_xdg_unset_home_unset_win_set)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/local/ACME/tron/cache");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE(path_get_app_dir_cache_xdg_unset_home_unset_win_unset)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), NULL));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_LocalAppData, alloca(MAX_PATH));
    strcat(buf, "\\ACME\\tron\\cache");
#else
    sprintf(buf, "%s/.cache/tron", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_config_xdg_set_home_set_win_set)
{
    test_int_success(env_set(LIT("XDG_CONFIG_HOME"), LIT("/home/foo/my_config")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_config/tron");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_config_xdg_unset_home_set_win_set)
{
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.config/tron");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE(path_get_app_dir_config_xdg_unset_home_unset_win_set)
{
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/baz/my_data/roaming")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/roaming/ACME/tron");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE(path_get_app_dir_config_xdg_unset_home_unset_win_unset)
{
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("APPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), NULL));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_RoamingAppData, alloca(MAX_PATH));
    strcat(buf, "\\ACME\\tron");
#else
    sprintf(buf, "%s/.config/tron", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_data_xdg_set_home_set_win_set)
{
    test_int_success(env_set(LIT("XDG_DATA_HOME"), LIT("/home/foo/my_data")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_data/tron");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_data_xdg_unset_home_set_win_set)
{
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.local/share/tron");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE(path_get_app_dir_data_xdg_unset_home_unset_win_set)
{
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/local/ACME/tron");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE(path_get_app_dir_data_xdg_unset_home_unset_win_unset)
{
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), NULL));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_LocalAppData, alloca(MAX_PATH));
    strcat(buf, "\\ACME\\tron");
#else
    sprintf(buf, "%s/.local/share/tron", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_log_xdg_set_home_set_win_set)
{
    test_int_success(env_set(LIT("XDG_CACHE_HOME"), LIT("/home/foo/my_cache")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_cache/tron/logs");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_log_xdg_unset_home_set_win_set)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.cache/tron/logs");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE(path_get_app_dir_log_xdg_unset_home_unset_win_set)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/local/ACME/tron/logs");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE(path_get_app_dir_log_xdg_unset_home_unset_win_unset)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), NULL));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_LocalAppData, alloca(MAX_PATH));
    strcat(buf, "\\ACME\\tron\\logs");
#else
    sprintf(buf, "%s/.cache/tron/logs", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_runtime_xdg_set_home_set_win_set)
{
    test_int_success(env_set(LIT("XDG_RUNTIME_DIR"), LIT("/home/foo/my_run")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar/")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_RUNTIME, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_run/tron");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_runtime_xdg_unset_home_set_win_set)
{
    test_int_success(env_unset(LIT("XDG_RUNTIME_DIR")));
    test_int_success(env_set(LIT("HOME"), LIT("/home/bar")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_RUNTIME, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/bar/.cache/tron/run");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE(path_get_app_dir_runtime_xdg_unset_home_unset_win_set)
{
    test_int_success(env_unset(LIT("XDG_RUNTIME_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/baz/my_data/local")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_RUNTIME, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/baz/my_data/local/ACME/tron/run");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE(path_get_app_dir_runtime_xdg_unset_home_unset_win_unset)
{
    test_int_success(env_unset(LIT("XDG_RUNTIME_DIR")));
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_RUNTIME, LIT("ACME"), LIT("tron"), NULL));
    
#ifdef _WIN32
    buf = _test_path_windows_folder(&FOLDERID_LocalAppData, alloca(MAX_PATH));
    strcat(buf, "\\ACME\\tron\\run");
#else
    sprintf(buf, "%s/.cache/run", getpwuid(getuid())->pw_dir);
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), buf);
    str_unref(cpath);
    path_free(path);
}

test_suite_ct test_suite_sys_path(void)
{
    return test_suite_new_with_cases("path"
        , test_case_new(path_get_user_home_env)
        , test_case_new(path_get_user_home_native)
        
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
        , test_case_new(path_get_app_dir_invalid_author1)
        , test_case_new(path_get_app_dir_invalid_author2)
        , test_case_new(path_get_app_dir_invalid_name1)
        , test_case_new(path_get_app_dir_invalid_name2)
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
        
        , test_case_new(path_get_app_dir_runtime_xdg_set_home_set_win_set)
        , test_case_new(path_get_app_dir_runtime_xdg_unset_home_set_win_set)
        , test_case_new_windows(path_get_app_dir_runtime_xdg_unset_home_unset_win_set)
        , test_case_new(path_get_app_dir_runtime_xdg_unset_home_unset_win_unset)
    );
}
