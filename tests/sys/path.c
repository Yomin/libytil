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

char *buf;
const char *cstr;
path_ct path;
str_const_ct cpath;


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
    SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, (buf = alloca(MAX_PATH)));
    cstr = buf;
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
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (buf = alloca(MAX_PATH)));
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
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (buf = alloca(MAX_PATH)));
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
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (buf = alloca(MAX_PATH)));
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
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (buf = alloca(MAX_PATH)));
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
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, (buf = alloca(MAX_PATH)));
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
        /*, test_case_new(path_get_user_dir_desktop_from_xdg_desktop_dir)
        , test_case_new(path_get_user_dir_documents_from_xdg_documents_dir)
        , test_case_new(path_get_user_dir_download_from_xdg_download_dir)
        , test_case_new(path_get_user_dir_music_from_xdg_music_dir)
        , test_case_new(path_get_user_dir_pictures_from_xdg_pictures_dir)
        , test_case_new(path_get_user_dir_publicshare_from_xdg_publicshare_dir)
        , test_case_new(path_get_user_dir_templates_from_xdg_templates_dir)
        , test_case_new(path_get_user_dir_videos_from_xdg_videos_dir)*/
        
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
