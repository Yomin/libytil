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

#include "fs.h"
#include <ytil/test/test.h>
#include <ytil/sys/env.h>
#include <ytil/sys/fs.h>
#include <ytil/sys/path.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef _WIN32
#   define mkdir(path, mode) mkdir(path)
#else
#   define mkdir(path, mode) mkdir(path, mode)
#endif

static path_ct path1, path2;
static str_ct str1, str2;


TEST_SETUP(path_new)
{
    test_ptr_success(path1 = path_new(LIT("/ytil_test"), PATH_STYLE_NATIVE));
}

TEST_SETUP(path_new2)
{
    test_ptr_success(path1 = path_new(LIT("/ytil_test1"), PATH_STYLE_NATIVE));
    test_ptr_success(path2 = path_new(LIT("/ytil_test2"), PATH_STYLE_NATIVE));
}

TEST_SETUP(path_new_drive)
{
    test_ptr_success(path1 = path_new(LIT("c:\\foo"), PATH_STYLE_WINDOWS));
}

TEST_SETUP(path_new_root_and_drive)
{
    test_ptr_success(path1 = path_new(LIT("/ytil_test"), PATH_STYLE_NATIVE));
    test_ptr_success(path2 = path_new(LIT("c:\\foo"), PATH_STYLE_WINDOWS));
}

TEST_TEARDOWN(path_free)
{
    path_free(path1);
}

TEST_TEARDOWN(path_free2)
{
    path_free(path1);
    path_free(path2);
}

TEST_SETUP(fs_mkfile)
{
    FILE *fp;
    
    test_ptr_success(path1 = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_void(env_free());
    
    test_ptr_success(path_append(path1, LIT("ytil_test_file"), PATH_STYLE_NATIVE));
    test_ptr_success(str1 = path_get(path1, PATH_STYLE_NATIVE));
    
    test_ptr_success_errno(fp = fopen(str_c(str1), "wb+"));
    test_int_success_errno(fprintf(fp, "YTIL TEST\n"));
    test_int_success_errno(fclose(fp));
}

TEST_TEARDOWN(fs_rmfile)
{
    unlink(str_c(str1));
    str_unref(str1);
    path_free(path1);
}

TEST_CASE_ABORT(fs_stat_invalid_path1)
{
    fs_stat_st fst;
    
    fs_stat(NULL, FS_LINK_FOLLOW, &fst);
}

#ifndef _WIN32
TEST_CASE_FIXTURE(fs_stat_invalid_path2, path_new_drive, path_free)
{
    fs_stat_st fst;
    
    test_ptr_error(fs_stat(path1, FS_LINK_FOLLOW, &fst), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_FIXTURE_ABORT(fs_stat_invalid_mode, path_new, path_free)
{
    fs_stat_st fst;
    
    fs_stat(path1, 999, &fst);
}

TEST_CASE_FIXTURE_ABORT(fs_stat_invalid_fst, path_new, path_free)
{
    fs_stat(path1, FS_LINK_FOLLOW, NULL);
}

TEST_CASE(fs_stat_not_found)
{
    fs_stat_st fst;
    
    test_ptr_success(path1 = path_new(LIT("/ytil_test"), PATH_STYLE_NATIVE));
    test_ptr_error(fs_stat(path1, FS_LINK_FOLLOW, &fst), E_FS_NOT_FOUND);
    path_free(path1);
}

TEST_CASE_FIXTURE(fs_stat_file, fs_mkfile, fs_rmfile)
{
    fs_stat_st fst;
    struct stat st;
    
    test_int_success_errno(stat(str_c(str1), &st));
    
    test_ptr_success(fs_stat(path1, FS_LINK_FOLLOW, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
    test_int_eq(fst.size, st.st_size);
    test_int_eq(fst.uid, st.st_uid);
    test_int_eq(fst.gid, st.st_gid);
    test_int_eq(fst.atime, st.st_atime);
    test_int_eq(fst.mtime, st.st_mtime);
    test_int_eq(fst.ctime, st.st_ctime);
}

#ifndef _WIN32
TEST_CASE_FIXTURE(fs_stat_link, fs_mkfile, fs_rmfile)
{
    fs_stat_st fst;
    
    test_ptr_success(path_drop(path1, 1));
    test_ptr_success(path_append(path1, LIT("ytil_test_link"), PATH_STYLE_NATIVE));
    test_ptr_success(str2 = path_get(path1, PATH_STYLE_NATIVE));
    test_int_maybe_errno(symlink(str_c(str1), str_c(str2)), EEXIST);
    
    test_ptr_success(fs_stat(path1, FS_LINK_FOLLOW, &fst));
    test_uint_eq(fst.type, FS_TYPE_REGULAR);
    test_ptr_success(fs_stat(path1, FS_LINK_NOFOLLOW, &fst));
    test_uint_eq(fst.type, FS_TYPE_LINK);
    
    unlink(str_c(str2));
    str_unref(str2);
}
#endif

TEST_CASE(fs_stat_dir)
{
    fs_stat_st fst;
    
    test_ptr_success(path1 = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_void(env_free());
    
    test_ptr_success(path_append(path1, LIT("ytil_test_dir"), PATH_STYLE_NATIVE));
    test_ptr_success(str1 = path_get(path1, PATH_STYLE_NATIVE));
    test_int_maybe_errno(mkdir(str_c(str1), S_IRWXU|S_IRWXG), EEXIST);
    
    test_ptr_success(fs_stat(path1, FS_LINK_FOLLOW, &fst));
    test_uint_eq(fst.type, FS_TYPE_DIRECTORY);
    
    test_int_success_errno(rmdir(str_c(str1)));
    str_unref(str1);
    path_free(path1);
}

typedef struct fs_walk_test
{
    size_t dir_pre, dir_post, file;
} fs_walk_test_st;

static int _test_fs_walk(fs_walk_type_id type, path_const_ct file, size_t depth, fs_stat_st *info, void *ctx)
{
    fs_walk_test_st *test = ctx;
    
    switch(type)
    {
    case FS_WALK_FILE:          test->file++; break;
    case FS_WALK_DIR_PRE:       test->dir_pre++; break;
    case FS_WALK_DIR_POST:      test->dir_post++; break;
    case FS_WALK_DIR_ERROR:
    case FS_WALK_STAT_ERROR:    error_pass(); return -1;
    default:                    abort();
    }
    
    return 0;
}

TEST_FUNCTION(void, fs_mkdir, path_ct base, const char *dir, int mode, bool mkfile, size_t drop)
{
    str_ct str;
    FILE *fp;
    
    test_ptr_success(path_append_c(base, dir, PATH_STYLE_POSIX));
    test_ptr_success(str = path_get(base, PATH_STYLE_NATIVE));
    test_int_maybe_errno(mkdir(str_c(str), mode), EEXIST);
    test_int_success_errno(chmod(str_c(str), mode));
    str_unref(str);
    
    if(mkfile)
    {
        test_ptr_success(path_append_c(base, "file", PATH_STYLE_POSIX));
        test_ptr_success(str = path_get(base, PATH_STYLE_NATIVE));
        test_ptr_maybe_errno(fp = fopen(str_c(str), "w"), EEXIST);
        test_int_success_errno(fclose(fp));
        str_unref(str);
    }
    
    test_ptr_success(path_drop(base, drop+(mkfile?1:0)));
}

TEST_FUNCTION(void, fs_mktree, bool blocker)
{
    test_ptr_success(path1 = path_get_base_dir(PATH_BASE_DIR_TMP));
    
    test_call(fs_mkdir, path1, "ytil_test_dir2", S_IRWXU, false, 0);
    test_call(fs_mkdir, path1, "foo", S_IRWXU, false, 0);
    test_call(fs_mkdir, path1, "foo1", S_IRWXU, true, 1);
    test_call(fs_mkdir, path1, "foo2", S_IRWXU, true, 1);
    test_call(fs_mkdir, path1, "foo3", S_IRWXU, true, 2);
    
    if(blocker)
        test_call(fs_mkdir, path1, "bar", 0, false, 1);
    else
    {
        test_call(fs_mkdir, path1, "bar", S_IRWXU, false, 0);
        test_call(fs_mkdir, path1, "bar1", S_IRWXU, true, 1);
        test_call(fs_mkdir, path1, "bar2", S_IRWXU, true, 1);
        test_call(fs_mkdir, path1, "bar3", S_IRWXU, true, 2);
    }
    
    test_call(fs_mkdir, path1, "baz", S_IRWXU, false, 0);
    test_call(fs_mkdir, path1, "baz1", S_IRWXU, true, 1);
    test_call(fs_mkdir, path1, "baz2", S_IRWXU, true, 1);
    test_call(fs_mkdir, path1, "baz3", S_IRWXU, true, 2);
}

TEST_SETUP(fs_mktree)
{
    test_call(fs_mktree, false);
}

TEST_SETUP(fs_mktree_blocker)
{
    test_call(fs_mktree, true);
}

static int _test_fs_rmtree_error(fs_walk_type_id type, path_const_ct file, size_t depth, fs_stat_st *info, void *ctx)
{
    void *_test_case_ctx = ctx;
    str_ct str;
    
    if(!error_check(0, E_FS_NOT_FOUND) && (str = path_get(file, PATH_STYLE_NATIVE)))
    {
        if(!chmod(str_c(str), S_IRWXU) && !rmdir(str_c(str)))
            return str_unref(str), 0;
        
        test_msg_backtrace("failed to remove '%s'", str_c(str));
        str_unref(str);
    }
    
    return 0;
}

TEST_TEARDOWN(fs_rmtree)
{
    fs_remove(path1, _test_fs_rmtree_error, TEST_CTX);
    env_free();
    path_free(path1);
}

TEST_CASE_ABORT(fs_walk_invalid_path1)
{
    fs_walk(NULL, 0, FS_LINK_FOLLOW, _test_fs_walk, NULL);
}

#ifndef _WIN32
TEST_CASE_FIXTURE(fs_walk_invalid_path2, path_new_drive, path_free)
{
    test_int_error(fs_walk(path1, 0, FS_LINK_FOLLOW, _test_fs_walk, NULL), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_FIXTURE_ABORT(fs_walk_invalid_link_mode, path_new, path_free)
{
    fs_walk(path1, 0, 999, _test_fs_walk, NULL);
}

TEST_CASE_FIXTURE_ABORT(fs_walk_invalid_callback, path_new, path_free)
{
    fs_walk(path1, 0, FS_LINK_FOLLOW, NULL, NULL);
}

TEST_CASE(fs_walk_not_found)
{
    test_ptr_success(path1 = path_new(LIT("/ytil_test"), PATH_STYLE_NATIVE));
    test_int_error(fs_walk(path1, 0, FS_LINK_FOLLOW, _test_fs_walk, NULL), E_FS_CALLBACK);
    test_error(1, E_FS_NOT_FOUND);
    path_free(path1);
}

TEST_CASE_FIXTURE(fs_walk_depth0, fs_mktree, fs_rmtree)
{
    fs_walk_test_st test = {0};
    
    test_int_success(fs_walk(path1, 0, FS_LINK_FOLLOW, _test_fs_walk, &test));
    test_uint_eq(test.dir_pre, 1);
    test_uint_eq(test.dir_post, 1);
    test_uint_eq(test.file, 0);
}

TEST_CASE_FIXTURE(fs_walk_depth1, fs_mktree, fs_rmtree)
{
    fs_walk_test_st test = {0};
    
    test_int_success(fs_walk(path1, 1, FS_LINK_FOLLOW, _test_fs_walk, &test));
    test_uint_eq(test.dir_pre, 4);
    test_uint_eq(test.dir_post, 4);
    test_uint_eq(test.file, 0);
}

TEST_CASE_FIXTURE(fs_walk_recursive, fs_mktree, fs_rmtree)
{
    fs_walk_test_st test = {0};
    
    test_int_success(fs_walk(path1, -1, FS_LINK_FOLLOW, _test_fs_walk, &test));
    test_uint_eq(test.dir_pre, 13);
    test_uint_eq(test.dir_post, 13);
    test_uint_eq(test.file, 9);
}

TEST_CASE_FIXTURE_ABORT(fs_move_invalid_dst1, path_new, path_free)
{
    fs_move(NULL, path1, FS_COPY_REPLACE);
}

#ifndef _WIN32
TEST_CASE_FIXTURE(fs_move_invalid_dst2, path_new_root_and_drive, path_free2)
{
    test_int_error(fs_move(path2, path1, FS_COPY_REPLACE), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_FIXTURE_ABORT(fs_move_invalid_src1, path_new, path_free)
{
    fs_move(path1, NULL, FS_COPY_REPLACE);
}

#ifndef _WIN32
TEST_CASE_FIXTURE(fs_move_invalid_src2, path_new_root_and_drive, path_free2)
{
    test_int_error(fs_move(path1, path2, FS_COPY_REPLACE), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_FIXTURE_ABORT(fs_move_invalid_mode, path_new2, path_free2)
{
    fs_move(path1, path2, 999);
}

TEST_CASE_FIXTURE(fs_move_not_found, path_new2, path_free2)
{
    test_int_error(fs_move(path1, path2, FS_COPY_REPLACE), E_FS_NOT_FOUND);
}

TEST_CASE_ABORT(fs_remove_invalid_path1)
{
    fs_remove(NULL, NULL, NULL);
}

#ifndef _WIN32
TEST_CASE_FIXTURE(fs_remove_invalid_path2, path_new_drive, path_free)
{
    test_int_error(fs_remove(path1, NULL, NULL), E_FS_INVALID_PATH);
}
#endif

TEST_CASE_FIXTURE(fs_remove_not_found, path_new, path_free)
{
    test_int_error(fs_remove(path1, NULL, NULL), E_FS_NOT_FOUND);
}

TEST_CASE_FIXTURE(fs_remove_file, fs_mkfile, fs_rmfile)
{
    fs_stat_st fst;
    
    test_int_success(fs_remove(path1, NULL, NULL));
    test_ptr_error(fs_stat(path1, FS_LINK_NOFOLLOW, &fst), E_FS_NOT_FOUND);
}

TEST_CASE_FIXTURE(fs_remove_dir, fs_mktree, fs_rmtree)
{
    fs_stat_st fst;
    
    test_int_success(fs_remove(path1, NULL, NULL));
    test_ptr_error(fs_stat(path1, FS_LINK_NOFOLLOW, &fst), E_FS_NOT_FOUND);
}

static int _test_fs_remove_error(fs_walk_type_id type, path_const_ct file, size_t depth, fs_stat_st *info, void *ctx)
{
    str_ct *blocker = ctx;
    
    *blocker = path_basename(file, PATH_STYLE_NATIVE);
    
    return error_pass(), -1;
}

TEST_CASE_FIXTURE(fs_remove_fail, fs_mktree_blocker, fs_rmtree)
{
    str_ct blocker;
    
    test_int_error(fs_remove(path1, _test_fs_remove_error, &blocker), E_FS_CALLBACK);
    test_error(1, E_FS_ACCESS_DENIED);
    
    test_str_eq(str_c(blocker), "bar");
    str_unref(blocker);
}

test_suite_ct test_suite_sys_fsys(void)
{
    return test_suite_new_with_cases("fs"
        , test_case_new(fs_stat_invalid_path1)
        , test_case_new_unix(fs_stat_invalid_path2)
        , test_case_new(fs_stat_invalid_mode)
        , test_case_new(fs_stat_invalid_fst)
        , test_case_new(fs_stat_not_found)
        , test_case_new(fs_stat_file)
        , test_case_new_unix(fs_stat_link)
        , test_case_new(fs_stat_dir)
        
        , test_case_new(fs_walk_invalid_path1)
        , test_case_new_unix(fs_walk_invalid_path2)
        , test_case_new(fs_walk_invalid_link_mode)
        , test_case_new(fs_walk_invalid_callback)
        , test_case_new(fs_walk_not_found)
        , test_case_new(fs_walk_depth0)
        , test_case_new(fs_walk_depth1)
        , test_case_new(fs_walk_recursive)
        
        , test_case_new(fs_move_invalid_dst1)
        , test_case_new_unix(fs_move_invalid_dst2)
        , test_case_new(fs_move_invalid_src1)
        , test_case_new_unix(fs_move_invalid_src2)
        , test_case_new(fs_move_invalid_mode)
        , test_case_new(fs_move_not_found)
        
        , test_case_new(fs_remove_invalid_path1)
        , test_case_new_unix(fs_remove_invalid_path2)
        , test_case_new(fs_remove_not_found)
        , test_case_new(fs_remove_file)
        , test_case_new(fs_remove_dir)
        , test_case_new(fs_remove_fail)
    );
}
