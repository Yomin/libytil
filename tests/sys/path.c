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
#   include <ShlObj.h>
#else
#   include <unistd.h>
#   include <pwd.h>
#endif

char *tmp;
const char *cstr;
path_ct path;
str_const_ct cpath;


TEST_CASE_ABORT(path_get_user_dir_invalid_ident)
{
    path_get_user_dir(999);
}

TEST_CASE(path_get_user_dir_home_from_home)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_HOME));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo");
    str_unref(cpath);
    path_free(path);
}

#ifdef _WIN32
TEST_CASE(path_get_user_dir_home_from_userprofile)
{
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_set(LIT("USERPROFILE"), LIT("c:\\users\\foo")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_HOME));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_WINDOWS));
    test_str_eq(str_c(cpath), "c:\\users\\foo");
    str_unref(cpath);
    path_free(path);
}
#endif

TEST_CASE(path_get_user_dir_home_native)
{
    test_int_success(env_unset(LIT("HOME")));
    test_int_success(env_unset(LIT("USERPROFILE")));
    test_ptr_success(path = path_get_user_dir(PATH_USER_DIR_HOME));
    
#ifdef _WIN32
    SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, (tmp = alloca(MAX_PATH)));
    cstr = tmp;
#else
    cstr = getpwuid(getuid())->pw_dir;
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), cstr);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE_ABORT(path_get_app_dir_invalid_ident)
{
    path_get_app_dir(999, LIT("ACME"), LIT("tron"), NULL);
}

TEST_CASE_ABORT(path_get_app_dir_invalid_author)
{
    path_get_app_dir(PATH_APP_DIR_CACHE, NULL, LIT("tron"), NULL);
}

TEST_CASE_ABORT(path_get_app_dir_invalid_name)
{
    path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), NULL, NULL);
}

TEST_CASE(path_get_app_dir_cache_from_xdg_cache_home)
{
    test_int_success(env_set(LIT("XDG_CACHE_HOME"), LIT("/home/foo/my_cache")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_cache/tron");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_cache_from_localappdata)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/foo/appdata/glocal")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/foo/appdata/glocal/ACME/tron/cache");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_cache_native)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CACHE, LIT("ACME"), LIT("tron"), NULL));
    
#ifdef _WIN32
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (tmp = alloca(MAX_PATH)));
    cstr = strcat(tmp, "\\ACME\\tron\\cache");
#else
    cstr = "/home/foo/.cache/tron";
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), cstr);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_config_from_xdg_config_home)
{
    test_int_success(env_set(LIT("XDG_CONFIG_HOME"), LIT("/home/foo/my_config")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_config/tron");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_config_from_appdata)
{
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_set(LIT("APPDATA"), LIT("/users/foo/appdata/goofing")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/foo/appdata/goofing/ACME/tron");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_config_native)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_int_success(env_unset(LIT("APPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_CONFIG, LIT("ACME"), LIT("tron"), NULL));
    
#ifdef _WIN32
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (tmp = alloca(MAX_PATH)));
    cstr = strcat(tmp, "\\ACME\\tron");
#else
    cstr = "/home/foo/.config/tron";
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), cstr);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_data_from_xdg_data_home)
{
    test_int_success(env_set(LIT("XDG_DATA_HOME"), LIT("/home/foo/my_data")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_data/tron");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_data_from_localappdata)
{
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/foo/appdata/glocal")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/foo/appdata/glocal/ACME/tron");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_data_native)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_DATA, LIT("ACME"), LIT("tron"), NULL));
    
#ifdef _WIN32
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (tmp = alloca(MAX_PATH)));
    cstr = strcat(tmp, "\\ACME\\tron");
#else
    cstr = "/home/foo/.local/share/tron";
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), cstr);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_log_from_xdg_cache_home)
{
    test_int_success(env_set(LIT("XDG_CACHE_HOME"), LIT("/home/foo/my_cache")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_cache/tron/logs");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_log_from_localappdata)
{
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/foo/appdata/glocal")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/foo/appdata/glocal/ACME/tron/logs");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_log_native)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_LOG, LIT("ACME"), LIT("tron"), NULL));
    
#ifdef _WIN32
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (tmp = alloca(MAX_PATH)));
    cstr = strcat(tmp, "\\ACME\\tron\\logs");
#else
    cstr = "/home/foo/.cache/tron/logs";
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), cstr);
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_runtime_from_xdg_runtime_dir)
{
    test_int_success(env_set(LIT("XDG_RUNTIME_DIR"), LIT("/home/foo/my_sockets")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_RUNTIME, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/home/foo/my_sockets/tron");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_runtime_from_localappdata)
{
    test_int_success(env_unset(LIT("XDG_RUNTIME_DIR")));
    test_int_success(env_set(LIT("LOCALAPPDATA"), LIT("/users/foo/appdata/glocal")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_RUNTIME, LIT("ACME"), LIT("tron"), NULL));
    test_ptr_success(cpath = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(cpath), "/users/foo/appdata/glocal/ACME/tron/run");
    str_unref(cpath);
    path_free(path);
}

TEST_CASE(path_get_app_dir_runtime_native)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_int_success(env_unset(LIT("XDG_RUNTIME_DIR")));
    test_int_success(env_unset(LIT("LOCALAPPDATA")));
    test_ptr_success(path = path_get_app_dir(PATH_APP_DIR_RUNTIME, LIT("ACME"), LIT("tron"), NULL));
    
#ifdef _WIN32
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (tmp = alloca(MAX_PATH)));
    cstr = strcat(tmp, "\\ACME\\tron\\run");
#else
    cstr = "/home/foo/.cache/tron/run";
#endif
    
    test_ptr_success(cpath = path_get(path, PATH_STYLE_NATIVE));
    test_str_eq(str_c(cpath), cstr);
    str_unref(cpath);
    path_free(path);
}

test_suite_ct test_suite_sys_path(void)
{
    return test_suite_new_with_cases("path"
        , test_case_new(path_get_user_dir_invalid_ident)
        , test_case_new(path_get_user_dir_home_from_home)
        , test_case_new_windows(path_get_user_dir_home_from_userprofile)
        , test_case_new(path_get_user_dir_home_native)
        
        /*, test_case_new(path_get_user_dir_desktop_from_xdg_desktop_dir)
        , test_case_new(path_get_user_dir_documents_from_xdg_documents_dir)
        , test_case_new(path_get_user_dir_download_from_xdg_download_dir)
        , test_case_new(path_get_user_dir_music_from_xdg_music_dir)
        , test_case_new(path_get_user_dir_pictures_from_xdg_pictures_dir)
        , test_case_new(path_get_user_dir_publicshare_from_xdg_publicshare_dir)
        , test_case_new(path_get_user_dir_templates_from_xdg_templates_dir)
        , test_case_new(path_get_user_dir_videos_from_xdg_videos_dir)*/
        
        , test_case_new(path_get_app_dir_invalid_ident)
        , test_case_new(path_get_app_dir_invalid_author)
        , test_case_new(path_get_app_dir_invalid_name)
        , test_case_new(path_get_app_dir_cache_from_xdg_cache_home)
        , test_case_new(path_get_app_dir_cache_from_localappdata)
        , test_case_new(path_get_app_dir_cache_native)
        , test_case_new(path_get_app_dir_config_from_xdg_config_home)
        , test_case_new(path_get_app_dir_config_from_appdata)
        , test_case_new(path_get_app_dir_config_native)
        , test_case_new(path_get_app_dir_data_from_xdg_data_home)
        , test_case_new(path_get_app_dir_data_from_localappdata)
        , test_case_new(path_get_app_dir_data_native)
        , test_case_new(path_get_app_dir_log_from_xdg_cache_home)
        , test_case_new(path_get_app_dir_log_from_localappdata)
        , test_case_new(path_get_app_dir_log_native)
        , test_case_new(path_get_app_dir_runtime_from_xdg_runtime_dir)
        , test_case_new(path_get_app_dir_runtime_from_localappdata)
        , test_case_new(path_get_app_dir_runtime_native)
    );
}
